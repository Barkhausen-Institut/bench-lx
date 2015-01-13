#!/usr/bin/php
<?php

if($argc != 3)
    exit("Usage: {$argv[0]} <13-cycles-res> <30-cycles-res>\n");

$a = file($argv[1]);
$b = file($argv[2]);

if(count($a) != count($b))
    exit("Invalid input");

function calc($a, $b) {
    $ai = preg_replace('/^.*?: /','',trim($a));
    $bi = preg_replace('/^.*?: /','',trim($b));

    ob_start();
    system("octave -q --eval 'A = [1, 13; 1, 30]; b = [$ai; $bi]; x = A \ b; nth_element(x, 2)'");
    $res = ob_get_contents();
    ob_end_clean();
    $baseline = preg_replace('/^ans =\s*/','',$res);
    return array($bi, $baseline);
}

function part($a,$b,$indices) {
    $lxnocache = '';
    $lx = '';
    foreach($indices as $i) {
        $res = calc($a[$i],$b[$i]);
        $lx .= $res[0] . ' ';
        $lxnocache .= sprintf("%.0f ",$res[1]);
    }

    echo $lxnocache . "\n";
    echo $lx . "\n";
}

$tasks = array(0, 17, 18, 19, 20);
$fs = array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

echo "memcpy syscall thread fork exec vfork\n";
part($a,$b,$tasks);

echo "read rd-memcpy readmmap readmmap-again readmmapcpy readmmapcpy-again ";
echo "readchksum readchksum-memcpy mmapchksum mmapchksum-again write wr-memcpy cp cp-memcpy cpmmap cpmmap-again\n";
part($a,$b,$fs);
?>