<?php

$file = "base.wxs";
if (!file_exists($file)) {
    echo "Fatal Error: $file doesn't exist!\n";
    return;
}

$output = fopen("windows.wxs", "w") or die("Unable to open file!");
$fp = @fopen($file, "r");
if ($fp) {
    $precedent = false;
    while (($line = fgets($fp, 4096)) !== false) {
        $pattern = '<Component Id';
        $pos = strpos($line, $pattern);
        $found = false;
        $files = array('<Directory Id="DEBUG"', '<Directory Id="RELEASE"', 'MAKEFILE');
        foreach ($files as &$pattern) {
            $pos = strpos($line, $pattern);
            if ($pos) {
                $found = true;
                $precedent = true;
            }
        }

        if (!$found) {
            $pattern = '</Component>';
            $pos = strpos($line, $pattern);
            if ($pos && $precedent) {

            } else {
                fwrite($output, $line);
            }

            $precedent = false;
        }
    }
    if (!feof($fp)) {
        echo "Error: unexpected fgets() fail\n";
    }
    fclose($fp);
    fclose($output);
}
?>
