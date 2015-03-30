#!/usr/bin/php
<?php

$i = 0;
define('IDX_SYSCALL',            $i++);
define('IDX_TASKCRT',            $i++);
define('IDX_PTHREAD',            $i++);
define('IDX_CLONE',              $i++);
define('IDX_FORK',               $i++);
define('IDX_EXEC',               $i++);
define('IDX_VEXEC',              $i++);
define('IDX_READ',               $i++);
define('IDX_READ_MEMCPY',        $i++);
define('IDX_READ_MMAP',          $i++);
define('IDX_READ_MMAP_AGAIN',    $i++);
define('IDX_READ_MMAP_CP',       $i++);
define('IDX_READ_MMAP_CP_AGAIN', $i++);
define('IDX_CHKSUM_READ',        $i++);
define('IDX_CHKSUM_READ_MEMCPY', $i++);
define('IDX_CHKSUM_MMAP',        $i++);
define('IDX_CHKSUM_MMAP_AGAIN',  $i++);
define('IDX_WRITE',              $i++);
define('IDX_WRITE_MEMCPY',       $i++);
define('IDX_COPY_RDWR',          $i++);
define('IDX_COPY_RDWR_MEMCPY',   $i++);
define('IDX_COPY_MMAP',          $i++);
define('IDX_COPY_MMAP_AGAIN',    $i++);
define('IDX_PIPE',               $i++);
define('IDX_PIPE_MEMCPY',        $i++);

if($argc != 5)
    exit("Usage: {$argv[0]} <13-cycles-res> <30-cycles-res> <m3-res> <m3-frag-res>\n");

$a = file($argv[1]);
$b = file($argv[2]);
$c = file($argv[3]);
$d = file($argv[4]);

if(count($a) != count($b))
    exit("Invalid input");

function calc($a, $b) {
    $ai = preg_replace('/^.*?: /','',trim($a));
    $bi = preg_replace('/^.*?: /','',trim($b));

    // we determine the time for running it without cache misses in the following way:
    // first, we define the time of an operation as:
    // T = C + n * M
    // that is, there are n cache-misses with M cycles each plus the remaining time C.
    // by running with 2 different times for M, we have 2 equations with 2 unknown variables.
    // thus, we can solve the linear equation.

    ob_start();
    // we have run it once with 13 cycles for M and once with 30 cycles.
    // thus, the equations are:
    // $ai = 1 * C + 13 * n
    // $bi = 1 * C + 30 * n
    // the right side is the matrix [1, 13; 1, 30].
    // we solve it by dividing it by [$ai; $bi] and take the second element to get C (opposite order)
    system("octave -q --eval 'A = [1, 13; 1, 30]; b = [$ai; $bi]; x = A \ b; nth_element(x, 2)'");
    $res = ob_get_contents();
    ob_end_clean();

    // now we have the baseline, i.e. the time without cache misses
    $baseline = preg_replace('/^ans =\s*/','',$res);
    return array($bi, $baseline);
}

function print_row($title, $cols) {
    echo "\t$title";
    foreach($cols as $col)
        echo " & " . $col;
    echo "\\\\\n";
    echo "\t\\hline\n";
}

function print_task_row($title, $idx) {
    global $a, $b, $c;
    $m3res = preg_replace('/^.*?: /','',trim($c[$idx]));
    $lxres = calc($a[$idx],$b[$idx]);
    print_row($title,array($m3res, $lxres[0], sprintf("%.0f",$lxres[1])));
}

// generate tasks table

echo "\\begin{tabular}{|l|c|c|c|}\n";
echo "\t\\hline\n";
echo "\t& \\textbf{M3} & \\textbf{Linux} & \\textbf{Linux (no \\$-misses)}\\\\\n";
echo "\t\\hline\n";
print_task_row("Syscall (noop)", IDX_SYSCALL);
print_task_row("Task creation", IDX_TASKCRT);
print_task_row("\\texttt{pthread\\_create}", IDX_PTHREAD);
print_task_row("\\texttt{clone}", IDX_CLONE);
print_task_row("\\texttt{fork}+\\texttt{waitpid}", IDX_FORK);
print_task_row("\\texttt{fork}+\\texttt{exec}+\\texttt{waitpid}", IDX_EXEC);
print_task_row("\\texttt{vfork}+\\texttt{exec}+\\texttt{waitpid}", IDX_VEXEC);
echo "\\end{tabular}\n";

echo "\n";
echo "\n";

function print_fs_row($title, $idx, $midx) {
    global $a, $b, $c, $d;
    $m3res = preg_replace('/^.*?: /', '', trim($c[$idx]));
    $m3fres = preg_replace('/^.*?: /', '', trim($d[$idx]));
    $lxres = calc($a[$idx],$b[$idx]);
    if($midx != -1) {
        $lxmres = calc($a[$midx], $b[$midx]);
        $lxmres[1] = sprintf("%.0f", $lxmres[1]);
    }
    else {
        $lxmres[0] = "-";
        $lxmres[1] = "-";
    }

    print_row($title,array(
        $m3res, $m3fres, $lxres[0], $lxmres[0], sprintf("%.0f", $lxres[1]), $lxmres[1]
    ));
}

// generate fs table

echo "\\begin{tabular}{|l|c|c|c|c|c|c|}\n";
echo "    \\hline\n";
echo "    & \\textbf{M3} & \\textbf{M3*} & \\textbf{Lx} & \\textbf{Lx (mcpy)} & \\textbf{Lx-\\$} & \\textbf{Lx-\\$ (mcpy)}\\\\\n";
echo "    \\hline\n";
print_fs_row("Read (\\texttt{read})", IDX_READ, IDX_READ_MEMCPY);
print_fs_row("Read (\\texttt{mmap})", IDX_READ_MMAP, -1);
print_fs_row("Read ag. (\\texttt{mmap})", IDX_READ_MMAP_AGAIN, -1);
print_fs_row("Read (\\texttt{mmap}+cp)", IDX_READ_MMAP_CP, -1);
print_fs_row("Read ag. (\\texttt{mmap}+cp)", IDX_READ_MMAP_CP_AGAIN, -1);
print_fs_row("Chksum (\\texttt{read})", IDX_CHKSUM_READ, IDX_CHKSUM_READ_MEMCPY);
print_fs_row("Chksum (\\texttt{mmap})", IDX_CHKSUM_MMAP, -1);
print_fs_row("Chksum ag. (\\texttt{mmap})", IDX_CHKSUM_MMAP_AGAIN, -1);
print_fs_row("Write (\\texttt{write})", IDX_WRITE, IDX_WRITE_MEMCPY);
print_fs_row("Copy (\\texttt{rd}+\\texttt{wr})", IDX_COPY_RDWR, IDX_COPY_RDWR_MEMCPY);
print_fs_row("Copy (\\texttt{mmap})", IDX_COPY_MMAP, -1);
print_fs_row("Copy ag. (\\texttt{mmap})", IDX_COPY_MMAP_AGAIN, -1);
print_fs_row("Pipe", IDX_PIPE, IDX_PIPE_MEMCPY);
echo "\\end{tabular}\n";
?>
