#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>


char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct event {
    u32 pid;
    char comm[16];
    char filename[256];
    int ret;
};

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} events SEC(".maps");

SEC("fexit/do_unlinkat")
int BPF_PROG(do_unlinkat_exit, int dfd, struct filename *name, long ret)
{
    struct event *e;
    e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) {
        return 0;
    }

    pid_t pid = bpf_get_current_pid_tgid() >> 32;
    e->pid = pid;

    bpf_get_current_comm(&e->comm, sizeof(e->comm));

    //const char *fname = BPF_CORE_READ(name, name);
    //bpf_probe_read_kernel(&e->filename, sizeof(e->filename), fname);
    e->filename = name->name

    e->ret = ret;

    bpf_ringbuf_submit(e, 0);

    return 0;
}