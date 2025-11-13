use libbpf_cargo::SkeletonBuilder;
use std::{env, path::PathBuf};

const SRC: &str = "src/bpf/hello.bpf.c";

fn main() {
    let mut out = PathBuf::from(env::var_os("OUT_DIR").unwrap());
    out.push("hello.skel.rs");
    SkeletonBuilder::new()
        .source(SRC)
        .build_and_generate(&out)
        .unwrap();
    println!("cargo:rerun-if-changed={SRC}");
}
