<?php

  
  $log = s60_log_open();
  
  $log->first_entry();
  
  echo 'first_entry(): ';var_dump( $log->first_entry() );
  

?>
