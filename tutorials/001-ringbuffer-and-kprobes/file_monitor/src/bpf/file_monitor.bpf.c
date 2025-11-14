#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>


char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct event {
    u32 pid;
    char comm[16];
    char filename[256];
    long ret;
};

// 256kb in size.
// event is about 280 bytes, so this buffer can hold 256kb / 280 bytes ~= 936 events.
// If events come faster than userspace loader can consume them, old events will be overwritten.
// If ringbuffer is too big, then kernel memory will be wasted.
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} events SEC(".maps");

SEC("kprobe/do_unlinkat")
int BPF_KPROBE(do_unlinkat, int dfd, struct filename *name)
{
    struct event *e;
    e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) {
        return 0;
    }

    e->pid = bpf_get_current_pid_tgid() >> 32;
    bpf_get_current_comm(&e->comm, sizeof(e->comm));
    BPF_CORE_READ_STR_INTO(&e->filename, name, name);
    e->ret = 0;

    bpf_ringbuf_submit(e, 0);
    return 0;
}

SEC("kretprobe/do_unlinkat")
int BPF_KRETPROBE(do_unlinkat_exit, long ret)
{
    struct event *e;
    e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) {
        return 0;
    }
    
    e->pid = bpf_get_current_pid_tgid() >> 32;
    bpf_get_current_comm(&e->comm, sizeof(e->comm));
    e->filename[0] = '\0';  // empty string since we don't have filename here.
    e->ret = ret;

    bpf_ringbuf_submit(e, 0);
    return 0;
}