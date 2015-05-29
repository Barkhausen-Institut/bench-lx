#!/usr/bin/php
<?php
$names = array(
    8 => "open",
    9 => "close",
    12 => "read",
    13 => "write",
    15 => "lseek",
    23 => "ftruncate",
    26 => "fsync",
    30 => "pread",
    31 => "pwrite",
    33 => "rename",
    38 => "unlink",
    39 => "rmdir",
    40 => "mkdir",
    46 => "stat",
    54 => "fstat",
    113 => "sendfile",
    114 => "sendfile64",
);
$numbers = array_flip($names);

if($argc != 3)
    exit("Usage: {$argv[0]} <strace> <timings>\n");

$strace = file($argv[1]);
$times = file($argv[2]);

$last = 0;
$timestamp = 0;
$i = 0;
$j = 0;
$seen_ioctl = false;
for(; isset($strace[$j]) && isset($times[$i]); $i++, $j++) {
    preg_match('/^\s*\[\s*\d+\]\s+(\d+)\s+(\d+)\s+(\d+)/', $times[$i], $ti);
    preg_match('/^(.+?)\(([^,]*)/', $strace[$j], $st);

    // ignore everything up to the first ioctl to ignore the dynamic linking stuff
    if(!$seen_ioctl && $st[1] != 'ioctl') {
        if(isset($numbers[$st[1]]) && @$names[$ti[1]] != $st[1]) {
            @file_put_contents('php://stderr',
                "Warning in line $i: syscalls do not match: " . $ti[1] . " vs. " . $st[1] . "\n");
            // use the same entry of $strace again
            $j--;
        }
        continue;
    }
    $seen_ioctl = true;

    $last = $ti[3];

    // ignore writes to stdout/stderr
    if($st[1] == 'write' && ($st[2] == '1' || $st[2] == '2'))
        continue;

    if(isset($numbers[$st[1]])) {
        if(@$names[$ti[1]] != $st[1])
            @exit("Warning in line $i: syscalls do not match: " . $ti[1] . " vs. " . $st[1] . "\n");

        // ignore waits of less than 1000 cycles.
        if($timestamp > 0 && ($ti[2] - $timestamp) > 1000)
            echo "_waituntil(" . ($ti[2] - $timestamp) . ") = 0\n";
        $timestamp = $ti[3];
    }
    else if($timestamp == 0)
        @$timestamp = $ti[2];
    echo $strace[$j];
}

if($timestamp > 0 && ($last - $timestamp) > 1000)
    echo "_waituntil(" . ($last - $timestamp) . ") = 0\n";
?>