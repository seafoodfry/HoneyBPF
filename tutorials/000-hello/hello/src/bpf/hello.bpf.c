/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
/*
See https://github.com/eunomia-bpf/bpf-developer-tutorial/blob/main/src/1-helloworld/README.md

This will show you:
- PID: which process
- UID: which user (0=root, 1000+=regular users typically)
- comm: process name (what binary is running)
- fd: file descriptor (0=stdin, 1=stdout, 2=stderr, 3+=files/sockets/etc)
- count: how many bytes trying to write
*/
#define BPF_NO_GLOBAL_DATA
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

typedef unsigned int u32;
typedef int pid_t;
typedef unsigned long size_t;

// Optional PID filter; set to 0 to disable filtering (default) so it watches all PIDs.
// Otherwise, set to a specific PID to filter on that process only.
const pid_t pid_filter = 0;

char LICENSE[] SEC("license") = "Dual BSD/GPL";

/*
$ sudo cat /sys/kernel/debug/tracing/events/syscalls/sys_enter_write/format
name: sys_enter_write
ID: 627
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:int __syscall_nr;	offset:8;	size:4;	signed:1;
	field:unsigned int fd;	offset:16;	size:8;	signed:0;
	field:const char * buf;	offset:24;	size:8;	signed:0;
	field:size_t count;	offset:32;	size:8;	signed:0;

print fmt: "fd: 0x%08lx, buf: 0x%08lx, count: 0x%08lx", ((unsigned long)(REC->fd)), ((unsigned long)(REC->buf)), ((unsigned long)(REC->count))
*/
struct trace_event_raw_sys_enter {
    unsigned short common_type;
    unsigned char common_flags;
    unsigned char common_preempt_count;
    int common_pid;
    int __syscall_nr;
    // The args[] array is just unsigned long - it's a generic container that holds whatever was passed in the registers
    // for that syscall. On 64-bit systems, unsigned long is 8 bytes, which matches pointer size.
    unsigned long args[6];  // This covers fd, buf, count, and any other syscall args
};

// You can find available tracepoints by running
// sudo ls /sys/kernel/debug/tracing/events/syscalls/
SEC("tp/syscalls/sys_enter_write")
int handle_tp(struct trace_event_raw_sys_enter *ctx)
{
    pid_t pid = bpf_get_current_pid_tgid() >> 32;
    if (pid_filter && pid != pid_filter)
        return 0;

    char comm[16];
    bpf_get_current_comm(&comm, sizeof(comm));

    u32 uid = bpf_get_current_uid_gid() & 0xFFFFFFFF;
    int fd = ctx->args[0];  // fd is an int (4 bytes)
    size_t count = ctx->args[2]; // count is size_t (8 bytes)

    // Multiple printk calls since there's a limited format string size.
    bpf_printk("WRITE: pid=%d uid=%d comm=%s", pid, uid, comm);
    bpf_printk("  fd=%d count=%lu bytes", fd, count);
    // sudo cat /sys/kernel/debug/tracing/trace_pipe 

    return 0;
}