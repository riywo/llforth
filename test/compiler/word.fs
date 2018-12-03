\ RUN: %{compile} %t && echo 'w1' | %t | FileCheck %s

: main

inbuf word
.
inbuf prints
bye

;

\ CHECK: 2 w1