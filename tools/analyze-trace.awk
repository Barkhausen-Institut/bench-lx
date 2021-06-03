#!/bin/awk -f

BEGIN {
    next_is_syscall = 0
    old_tid = 0
    cur_tid = 0
}

/ : handle_exception\+0x116 : / {
    match($0, /^([0-9]+):/, m)
    time = m[1] - sysc_start[cur_tid]
    printf("%s: %-20s: %12d ticks\n", cur_tid, cur_sysc[cur_tid], time)
    sysc_end[cur_tid] = m[1]
}

{
    if(next_is_syscall) {
        match($0, /^([0-9]+):.*@ 0x[a-f0-9]+ : ([a-z0-9A-Z_]+)/, m)
        cur_sysc[cur_tid] = m[2]
        sysc_start[cur_tid] = m[1]
        printf("%s: %-20s: %12d ticks @ %d\n", cur_tid, "userspace", m[1] - sysc_end[cur_tid], m[1])
        next_is_syscall = 0
    }
}

/ : handle_exception\+0x114 : / {
    next_is_syscall = 1
}

/ : __switch_to\+0xa : / {
    match($0, / : __switch_to\+0xa : .*A=0x([a-f0-9]+)/, m)
    old_tid = m[1]
}

/ : __switch_to\+0x3e : / {
    match($0, / : __switch_to\+0x3e : .*A=0x([a-f0-9]+)/, m)
    cur_tid = m[1]
    # print("Switched from", old_tid, " to", cur_tid)
}
