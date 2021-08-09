/*
 * Copyright (C) 2021, Tendsin Mende <tendsin.mende@mailbox.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of M3 (Microkernel-based SysteM for Heterogeneous Manycores).
 *
 * M3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * M3 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

use std::cmp;
use std::vec::Vec;
use std::net::UdpSocket;
use std::net::TcpListener;
use std::io::{Read, Write};

fn usage(name: &str) -> ! {
    println!("Usage: {} udp|tcp <ip> <port> <repeats>", name);
    std::process::exit(1);
}

fn main() {
    let args = std::env::args().map(|a| a.to_string()).collect::<Vec<_>>();
    if args.len() != 5 {
        usage(&args[0]);
    }

    let socktype = &args[1];
    let repeats = args[4].parse::<u32>().unwrap_or_else(|_| usage(&args[0]));

    let mut buffer = [0u8; 1024];

    let ep = format!("{}:{}", args[2], args[3]);

    if socktype == "udp" {
        println!("listening for UDP packets on {}", ep);
        let socket = UdpSocket::bind(ep).expect("bind failed");

        loop {
            let (amt, src) = socket.recv_from(&mut buffer).expect("recv_from failed");
            println!("received {} bytes from {}", amt, src);
        }
    }
    else {
        println!("waiting for TCP connection on {}", ep);
        let listener = TcpListener::bind(ep).expect("bind failed");

        // accept connections and process them serially
        let (mut stream, addr) = listener.accept().expect("accept failed");
        println!("accepted connection from {}", addr);

        // loop {
        //     let recv_len = stream
        //         .read(&mut buffer[..])
        //         .expect("receive failed");
        //     println!("Received {} bytes", recv_len);
        // }

        for _ in 0..repeats {
            let mut length_bytes = [0u8; 8];
            let recv_len = stream
                .read(&mut length_bytes)
                .expect("receive length failed");
            assert_eq!(recv_len, 8);

            let length = u64::from_le_bytes(length_bytes);
            println!("Expecting {} bytes", length);
            std::io::stdout().flush().unwrap();

            let mut rem = length;
            while rem > 0 {
                let amount = cmp::min(rem as usize, buffer.len());
                let recv_len = stream
                    .read(&mut buffer[0..amount])
                    .expect("receive failed");
                println!("Received {} -> {}/{} bytes", recv_len, length - rem, length);
                rem -= recv_len as u64;
            }

            println!("Received {} bytes", length);
            println!("Sending ACK...");
            std::io::stdout().flush().unwrap();

            stream.write(&[0u8]).expect("write failed");
        }

        println!("Shutting down");
    }

}
