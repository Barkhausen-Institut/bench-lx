#![feature(with_options)]

use std::io::{BufReader, Read, Write};
use std::net::TcpStream;

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
standalone_client <file> <ip> <port>

Example:
ycsb_standalone_client workload.wl 127.0.0.2 1337

<file> : Specifies the workloadfile thats being used. Must be generated via the ycsb_parser tool
<IP>   : Destination IP,
<PORT> : Destination port
"
    );
}

fn main() {
    // Parse arguments
    // workaround for riscv64-*-musl: apparently, the arguments don't work, so hardcode them
    // let args = vec!["whatever", "/bench/workload.wl", "127.0.0.1", "1337"];

    let args = std::env::args().map(|a| a.to_string()).collect::<Vec<_>>();
    if args.len() != 4 {
        print_help();
        return;
    }

    // Get file
    let workload_file = std::fs::File::open(&args[1]).expect("Could not open file");
    let mut workload = BufReader::new(workload_file);
    println!("Trying to connect to {}:{}", args[2], args[3]);
    let mut socket = TcpStream::connect(format!("{}:{}", args[2], args[3]))
        .expect("Failed to connect, is the server running?");

    // Now send requests until the workload file is empty
    let num_requests = importer::WorkloadHeader::load_from_file(&mut workload).number_of_operations;

    for idx in 0..num_requests {
        let new_req = importer::Package::load_as_bytes(&mut workload);
        let len = (new_req.len() as u32).to_be_bytes();
        socket.write(&len).unwrap();
        let written = socket.write(&new_req).expect("Failed to send request");
        assert!(written == new_req.len(), "Did not send whole request");

        if (idx + 1) % 16 == 0 {
            socket.read(&mut [0u8; 1]).expect("receive failed");
        }
    }

    socket.write(&(6 as u32).to_be_bytes()).unwrap();
    socket.write(b"ENDNOW").unwrap();

    println!("End");
}
