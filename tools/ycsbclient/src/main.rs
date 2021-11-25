use std::io::{Read, Write, BufReader};
use std::net::{UdpSocket, TcpStream};

mod importer;

const VERBOSE: bool = false;

fn usage(args: &[String]) {
    println!("Usage: {} tcp <remote_ip> <remote_port> <workload> <repeats>", args[0]);
    println!("Usage: {} udp <local_ip> <local_port>", args[0]);
    std::process::exit(1);
}

fn udp_receiver(local_ep: &str) {
    let socket = UdpSocket::bind(local_ep).expect("bind failed");

    let mut buf = vec![0u8; 1024];
    loop {
        let (amount, _) = socket.recv_from(&mut buf).expect("Receive failed");
        if VERBOSE {
            println!("Received {} bytes.", amount);
        }
    }
}

fn tcp_sender(remote_ep: &str, wl: &str, repeats: u32) {
    // Connect to server
    let mut socket = TcpStream::connect(remote_ep)
    .expect("Could not create TCP socket");

    for _ in 0..repeats {
        // open file
        let workload_file = std::fs::File::open(wl).expect("Could not open file");

        // Load workload info for the benchmark
        let mut workload_buffer = BufReader::new(workload_file);
        let workload_header = importer::WorkloadHeader::load_from_file(&mut workload_buffer);

        for _ in 0..workload_header.number_of_operations {
            let operation = importer::Package::load_as_bytes(&mut workload_buffer);
            debug_assert!(importer::Package::from_bytes(&operation).is_ok());

            if VERBOSE {
                println!("Sending operation...");
            }

            socket
                .write(&(operation.len() as u32).to_be_bytes())
                .expect("send failed");
            socket.write(&operation).expect("send failed");

            if VERBOSE {
                println!("Receiving response...");
            }

            let mut resp_bytes = [0u8; 8];
            socket
                .read(&mut resp_bytes)
                .expect("receive response header failed");
            let resp_len = u64::from_be_bytes(resp_bytes);

            if VERBOSE {
                println!("Expecting {} byte response.", resp_len);
            }

            let mut response = vec![0u8; resp_len as usize];
            let mut rem = resp_len as usize;
            while rem > 0 {
                let amount = socket
                    .read(&mut response[resp_len as usize - rem..])
                    .expect("receive response failed");
                rem -= amount;
            }

            if VERBOSE {
                println!("Got response.");
            }
        }

        let end_msg = b"ENDNOW";
        socket.write(&(end_msg.len() as u32).to_be_bytes()).unwrap();
        socket.write(end_msg).unwrap();
    }
}

fn main() {
    // Parse arguments
    let args = std::env::args().map(|a| a.to_string()).collect::<Vec<_>>();

    if args[1] == "udp" {
        if args.len() != 4 {
            usage(&args);
        }

        let local_ep = format!("{}:{}", args[2], args[3]);
        udp_receiver(&local_ep);
    }
    else {
        if args.len() != 6 {
            usage(&args);
        }

        let remote_ep = format!("{}:{}", args[2], args[3]);
        let repeats = args[5].parse::<u32>().expect("Failed to parse repeats");
        tcp_sender(&remote_ep, &args[4], repeats);
    }
}

