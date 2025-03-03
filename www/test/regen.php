<?php

// regenerate session, redirect back to test.php

session_start(); 
$_SESSION['visit_count'] = 0;
foreach ($_SESSION as $key => $value) {
    unset($_SESSION[ $key ]);
}
    
unset($_SESSION);
session_regenerate_id();
$_SESSION['visit_count'] = 0;

$_SERVER['REDIRECT_STATUS'] = 302;
header("HTTP/1.1 302 Found", true , 302);
header("Location: test.php?message=done", true , 302);
exit;
