# Setup

To get the kernel version:
```bash
uname -r
```

Check for function-level BTF information:
```bash
grep "^int " <(sudo bpftool btf dump file /sys/kernel/btf/vmlinux format c) | wc -l
```