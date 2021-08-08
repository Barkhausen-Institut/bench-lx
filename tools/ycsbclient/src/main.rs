#![feature(with_options)]

use std::net::TcpListener;
use std::io::{BufReader, Read, Write};


mod importer;

// make the linker happy. maybe it's because we're using the riscv64-*-gnu toolchain instead of
// riscv64-*-musl? we can't use *-musl, because std is not available and building it fails m(
#[no_mangle]
pub extern "C" fn __res_init() -> i32 { 0 }

fn print_help() {
    println!(
        "
---- ycsb workload sender for M³'s key value store benchmark ----

USAGE:
standalone_client <file> <ip> <port> <acks> <repeats>

Example:
ycsb_standalone_client workload.wl 127.0.0.2 1337 true 4

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
    if args.len() != 6 {
        print_help();
        return;
    }

    let acks = args[4].parse::<bool>().expect("parsing ACK failed");
    let repeats = args[5].parse::<u32>().expect("parsing repeats failed");

    let ep = format!("{}:{}", args[2], args[3]);
    let listener = TcpListener::bind(ep).expect("bind failed");

    loop {
        println!("waiting for TCP connection on {}:{}", args[2], args[3]);

        // accept connections and process them serially
        let (mut socket, addr) = listener.accept().expect("accept failed");
        println!("accepted connection from {}", addr);

        for r in 0..repeats {
            // Get file
            let workload_file = std::fs::File::open(&args[1]).expect("Could not open file");
            let mut workload = BufReader::new(workload_file);

            // Now send requests until the workload file is empty
            let num_requests = importer::WorkloadHeader::load_from_file(&mut workload).number_of_operations;

            for idx in 0..num_requests {
                let new_req = importer::Package::load_as_bytes(&mut workload);
                let len = (new_req.len() as u32).to_be_bytes();
                socket.write(&len).unwrap();
                let written = socket.write(&new_req).expect("Failed to send request");
                assert!(written == new_req.len(), "Did not send whole request");

                if acks && (idx + 1) % 16 == 0 {
                    socket.read(&mut [0u8; 1]).expect("receive failed");
                }
            }

            if r + 1 < repeats {
                socket.write(&(6 as u32).to_be_bytes()).unwrap();
                socket.write(b"ENDRUN").unwrap();
            }
        }

        socket.write(&(6 as u32).to_be_bytes()).unwrap();
        socket.write(b"ENDNOW").unwrap();
    }
}
