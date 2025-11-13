use anyhow::Result;
use libbpf_rs::skel::{SkelBuilder, OpenSkel, Skel};
use std::mem::MaybeUninit;

mod hello {
    include!(concat!(env!("OUT_DIR"), "/hello.skel.rs"));
}

use hello::*;

fn main() -> Result<()> {
    let mut obj = MaybeUninit::uninit();
    let skel_builder = HelloSkelBuilder::default();
    let open_skel = skel_builder.open(&mut obj)?;
    let mut skel = open_skel.load()?;
    skel.attach()?;

    println!("Tracing write syscalls for 60 seconds...");
    println!("Check output with: sudo cat /sys/kernel/debug/tracing/trace_pipe");
    
    std::thread::sleep(std::time::Duration::from_secs(60));
    
    println!("Done tracing!");
    Ok(())
}