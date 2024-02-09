<?php

$file = "tupitube.wxs";
if (!file_exists($file)) {
    echo "Fatal Error: $file doesn't exist!\n";
    return;
}

$fp = @fopen($file, "r");
if ($fp) {
    $output = fopen("base.wxs", "w") or die("Unable to open file!");
    while (($line = fgets($fp, 4096)) !== false) {
        $pattern = '<Component Id';
        $pos = strpos($line, $pattern);
        if ($pos) {
            $old = array('>');
            $new = array(' Win64="yes">');
            $line = str_replace($old, $new, $line);
            fwrite($output, $line);
        } else {
           $pattern = '<Package';
           $pos = strpos($line, $pattern);
           if ($pos) {
               $old = array('Test file in a Product');
               $new = array('TupiTube Desk');
               $line = str_replace($old, $new, $line);

               $old = array('Simple test');
               $new = array('2D Animation Tool');
               $line = str_replace($old, $new, $line);

               $old = array('/>');
               $new = array('Platform="x64" />');
               $line = str_replace($old, $new, $line);

               fwrite($output, $line);
           } else {
               $pattern = '<Icon Id="DesktopIcon.exe"';
               $pos = strpos($line, $pattern);
               if ($pos) {
                   $old = array('tupitube\tupitube.desk.exe');
                   $new = array('icon\tupitube.ico');
                   $line = str_replace($old, $new, $line);

                   fwrite($output, $line);
               } else {
                   $pattern = '<Icon Id="StartMenuIcon.exe"';
                   $pos = strpos($line, $pattern);
                   if ($pos) {
                       $old = array('tupitube\tupitube.desk.exe');
                       $new = array('icon\tupitube.ico');
                       $line = str_replace($old, $new, $line);

                       fwrite($output, $line);
                   } else {
                       fwrite($output, $line);
                   }
               }
           }
        }
    }
    if (!feof($fp)) {
        echo "Error: unexpected fgets() fail\n";
    }
    fclose($fp);
    fclose($output);
}
?>
