use anyhow::Result;
use libbpf_rs::RingBufferBuilder;
use libbpf_rs::skel::{OpenSkel, Skel, SkelBuilder};
use plain::Plain;
use std::mem::MaybeUninit;
use std::time::Duration;

mod bpf {
    include!(concat!(env!("OUT_DIR"), "/file_monitor.skel.rs"));
}

use bpf::*;

#[repr(C)]
struct Event {
    pid: u32,
    comm: [u8; 16],
    filename: [u8; 256],
    ret: i64,
}

impl Default for Event {
    fn default() -> Self {
        Event {
            pid: 0,
            comm: [0; 16],
            filename: [0; 256],
            ret: 0,
        }
    }
}

unsafe impl Plain for Event {}

fn handle_event(data: &[u8]) -> i32 {
    let mut event = Event::default();
    plain::copy_from_bytes(&mut event, data).expect("data buffer was not the right size");

    let comm = std::str::from_utf8(&event.comm)
        .unwrap_or("?")
        .trim_end_matches(char::from(0));
    let filename = std::str::from_utf8(&event.filename)
        .unwrap_or("?")
        .trim_end_matches(char::from(0));

    println!("PID: {}, CMD: {}, FILE: {}", event.pid, comm, filename);
    0
}

fn main() -> Result<()> {
    let mut obj = MaybeUninit::uninit();
    let skel_builder = FileMonitorSkelBuilder::default();
    let open_skel = skel_builder.open(&mut obj)?;
    let mut skel = open_skel.load()?;
    skel.attach()?;

    println!("Monitoring file deletions... Press Ctrl+C to exit");

    let mut builder = RingBufferBuilder::new();
    builder.add(&skel.maps.events, handle_event)?;
    let ringbuf = builder.build()?;

    loop {
        ringbuf.poll(Duration::from_millis(100))?;
    }
}
