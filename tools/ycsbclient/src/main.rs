#![feature(with_options)]

use std::io::BufReader;
use std::net::UdpSocket;

mod importer;

fn print_help() {
    println!(
        "
---- ycsb workload sender for M³'s key value store benchmark ----

USAGE:
standalone_client <file> <local_ip> <local_port> <remote_ip> <remote_port> <repeats>

Example:
ycsb_standalone_client workload.wl 127.0.0.2 1337 127.0.0.1 1338 4

<file> : Specifies the workloadfile thats being used. Must be generated via the ycsb_parser tool
<IP>   : Destination IP,
<PORT> : Destination port
<acks> : whether to expect ACKs from the other side
"
    );
}

fn main() {
    // Parse arguments
    let args = std::env::args().map(|a| a.to_string()).collect::<Vec<_>>();
    if args.len() != 7 {
        print_help();
        return;
    }

    let _repeats = args[6].parse::<u32>().expect("parsing repeats failed");

    let local_ep = format!("{}:{}", args[2], args[3]);
    let remote_ep = format!("{}:{}", args[4], args[5]);
    let socket = UdpSocket::bind(local_ep).expect("bind failed");

    println!("Waiting for client...\n");
    socket.recv_from(&mut [0u8; 1]).expect("receive failed");

    loop {
        // Get file
        let workload_file = std::fs::File::open(&args[1]).expect("Could not open file");
        let mut workload = BufReader::new(workload_file);

        // Now send requests until the workload file is empty
        let num_requests = importer::WorkloadHeader::load_from_file(&mut workload).number_of_operations;

        for _ in 0..num_requests {
            let mut new_req = &importer::Package::load_as_bytes(&mut workload)[..];
            println!("Sending {} byte via UDP to {}", new_req.len(), remote_ep);
            while !new_req.is_empty() {
                let written = socket.send_to(&new_req, remote_ep.clone()).expect("sent failed");
                new_req = &new_req[written..];
            }
        }

        std::thread::sleep(std::time::Duration::from_millis(100));
    }
}
