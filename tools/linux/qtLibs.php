<?php

$source = "/home/user/Qt5.14.1/5.14.1/gcc_64/lib/";
$target = "Qt5.14.1";

$output = fopen('files.sh', 'w');
$handle = fopen("files.txt", "r");
if ($handle) {
    fwrite($output, "mkdir ".$target."/\n");
    while (($line = fgets($handle)) !== false) {
        fwrite($output, "cp -av ".$source.trim($line)." ".$target."/\n");
    }
    fclose($handle);
    fclose($output);
}
?>
