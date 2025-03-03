<?php
session_start();

if(empty($_SESSION['visit_count']))
    $_SESSION['visit_count'] = 1;
else 
    $_SESSION['visit_count'] += 1;


$cookieVisitCount = empty($_COOKIE['cvisit_count'])? 1 : ($_COOKIE['cvisit_count'] + 1);
setcookie("cvisit_count", $cookieVisitCount , 0);


// $session_save_path = ini_get('session.save_path');
// echo "The session save path is: " . $session_save_path . "<br />";
// $PWD = ini_get('PWD');
// echo "The PWD is: " . $PWD . "<br />";
// echo "session ID = " . session_id();
// $php_ini_path = php_ini_loaded_file();
// echo "<hr>";
// var_dump($php_ini_path);

?>
<!DOCTYPE html><html><head><title>PHP Page</title>

    <link rel="stylesheet" href="style.css"></head>
 <body>
 <!-- <pre><?php print_r($_SESSION); ?></pre>    -->
    <div class="container">

    <p style="text-align:right"><input type="button" value="regenerate session" class="button" onclick="document.location.assign('regen.php');"/>        </p>

        <h3 style="text-align:center">Session Visit count: <?php echo empty($_SESSION['visit_count']) ? "1" : $_SESSION['visit_count']; ?></h3>
        <h3 style="text-align:center">Cookie Visit count: <?php echo empty($_COOKIE['cvisit_count']) ? "1" : $_COOKIE['cvisit_count']; ?></h3>

        <form method="POST" action="test.php?action=check" enctype="multipart/form-data">    
        <h1>Dynamic PHP Page</h1>
        <div class="section">
                <h2>This is a dynamic page generated from PHP script.</h2>

                <h3>action: <span class="hl">test.php</span> with query string <span class="hl">action=check</span></h3>

                <div class="subject">First Name : </div>
                <div> <input type="text" name="first_name" value="<?php echo @$_POST['first_name']; ?>" placeholder="Random..."> </div>

                <div class="subject">Last Name : </div>
                <div><input type="text" name="last_name" value="<?php echo @$_POST['last_name']; ?>" placeholder="Dude"> </div>

                <div class="subject">File#1: </div>
                <div><input type="file" name="uploaded_file1" class="button"></div>

                <div class="subject">File#2:</div> 
                <div><input type="file" name="uploaded_file2" class="button"></div>

                <div>
                    <input type="submit" value="Submit" class="button"/>        
                </div>                
        </div>
    </form>
    <?php 

    if( count($_GET) > 0 || count($_POST) >0) {
    ?>

    <div class="section">
        <h3>$_GET</h3>
        <div class="section">
            <pre><?php print_r($_GET); ?></pre>
        </div>
        <h3>$_POST</h3>
        <div class="section">
            <pre><?php print_r($_POST); ?></pre>
        </div>
        <h3>$_FILES</h3>
        <div class="section">
            <pre><?php print_r($_FILES); ?></pre>
        </div>
    </div>
    <?php
    }

    ?>
    </div>
    <h3 style="text-align:center">generated on <?php print date("Y-m-d H:i:s"); ?></h3>
    

</body></html>

