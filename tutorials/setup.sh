#!/bin/bash
set -euo pipefail
set -x

# FUSE (Filesystem in Userspace) lets regular programs mount filesystems without needing kernel modules or root privileges.
# It's basically a way for non-privileged code to act like a filesystem.
# AppImage is a portable app format for Linux - it's like a self-contained executable that bundles an app and all its dependencies
# into one file. You download it, make it executable, and run it without installing anything.
# AppImages use FUSE under the hood to mount themselves as a temporary filesystem when you run them,
# which is why ecc needs FUSE installed. The AppImage mounts itself, runs the program inside, then unmounts when done.
#
# bpftool is used to create vmlinux.h files from the kernel's BTF data.
sudo dnf install -y wget llvm clang fuse3 bpftool

(
    cd /tmp && \
    mkdir -p eunomia-bpf && \
    cd eunomia-bpf && \
    wget https://github.com/eunomia-bpf/eunomia-bpf/releases/download/v1.0.27/ecc-aarch64 && \
    wget https://github.com/eunomia-bpf/eunomia-bpf/releases/download/v1.0.27/ecli-aarch64 && \
    chmod +x ecc-aarch64 ecli-aarch64 && \
    sudo mv ecc-aarch64 /usr/local/bin/ecc && \
    sudo mv ecli-aarch64 /usr/local/bin/ecli
)

# Install Rust.
# See https://rust-lang.org/tools/install/
# The -s -- part tells sh that everything after it should be passed as arguments to the script.
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y