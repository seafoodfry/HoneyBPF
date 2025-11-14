/*
 * BPF Verifier and Memory Types
 * ==============================
 * 
 * The BPF verifier tracks the "type" of every pointer to ensure memory safety.
 * When you call bpf_map_update_elem(), the verifier checks that the value pointer
 * (3rd argument, stored in register R3) is one of these trusted types:
 *   - fp (frame pointer / stack memory)
 *   - pkt (packet data)
 *   - map_key
 *   - map_value
 * 
 * Memory from bpf_ringbuf_reserve() has type "alloc_mem" - dynamically allocated
 * memory that the verifier doesn't trust for map operations. This is because:
 *   1. The verifier can't statically analyze what's in that memory
 *   2. It could be freed/reused at any time
 *   3. Maps expect stable, verifiable pointers
 * 
 * How to debug verifier errors:
 * ------------------------------
 * Look for the failed instruction in the verifier log:
 *   21: (85) call bpf_map_update_elem#2
 *   R3 type=alloc_mem expected=fp, pkt, pkt_meta, map_key, map_value
 * 
 * This tells you:
 *   - Instruction 21 is calling bpf_map_update_elem
 *   - Register R3 (3rd arg = value pointer) has type "alloc_mem"
 *   - Verifier expected one of: fp, pkt, map_key, map_value
 * 
 * Trace backwards through the log to find where R3 came from:
 *   19: (bf) r3 = r8        ; copy from R8
 *   14: (07) r8 += 20       ; R8 offset by 20 bytes
 *   13: (bf) r8 = r6        ; copy from R6
 *   6:  (85) call bpf_ringbuf_reserve  ; R6 came from ringbuf_reserve
 * 
 * Solution: Copy to stack first (stack has type "fp"), then pass stack pointer
 * to bpf_map_update_elem(). The verifier trusts stack memory.
 */
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

// 256kb in size.
// event is about 280 bytes, so this buffer can hold 256kb / 280 bytes ~= 936 events.
// If events come faster than userspace loader can consume them, old events will be overwritten.
// If ringbuffer is too big, then kernel memory will be wasted.
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} events SEC(".maps");

struct call_key {
    u64 pid_tgid;  // u32 is good enough for pid alone.
    u64 kstack_ptr;
};

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 10 * 1024);
    __type(key, struct call_key);
    __type(value, char[256]); // filename.
} inflight SEC(".maps");


SEC("kprobe/do_unlinkat")
int BPF_KPROBE(do_unlinkat, int dfd, struct filename *name)
{
    // Get all the PID, TGID, and kernel stack stuff.
    u64 pid_tgid = bpf_get_current_pid_tgid();
    struct call_key key = {
        .pid_tgid = pid_tgid,
        .kstack_ptr = (u64)PT_REGS_SP(ctx),  // Use stack pointer, not ctx itself
    };

    // Read the filename and put it in a stack variable.
    // The verifier didn't like it when we tried to pass it directly to the map.
    char filename[256] = {};
    const char *fname = BPF_CORE_READ(name, name);
    bpf_probe_read_kernel(&filename, sizeof(filename), fname);

    // Create the event.
    /*
    struct event *e;
    e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) {
        return 0;
    }

    e->pid = pid_tgid >> 32;
    bpf_get_current_comm(&e->comm, sizeof(e->comm));
    __builtin_memcpy(&e->filename, &filename, sizeof(e->filename));
    e->ret = 0;
    */

    // Store the filename for the given pid, so that we can retrieve it in the exit probe.
    bpf_map_update_elem(&inflight, &key, &filename, BPF_ANY);

    ///bpf_ringbuf_submit(e, 0);
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
    
    u64 pid_tgid = bpf_get_current_pid_tgid();
    struct call_key key ={
        .pid_tgid = pid_tgid,
        .kstack_ptr = (u64)PT_REGS_SP(ctx),  // Use stack pointer, not ctx itself
    };

    e->pid = pid_tgid >> 32;
    bpf_get_current_comm(&e->comm, sizeof(e->comm));
    e->ret = ret;

    char *filename;
    filename = bpf_map_lookup_elem(&inflight, &key);
    if (filename) {
        __builtin_memcpy(&e->filename, filename, sizeof(e->filename));
        bpf_map_delete_elem(&inflight, &key);
    } else {
        e->filename[0] = '\0';  // empty string since we don't have filename here.
    }

    bpf_ringbuf_submit(e, 0);
    return 0;
}