\ RUN: %{compile} %t && echo '999999' | %t | FileCheck %s

: main

inbuf word
inbuf number
.
bye

;

\ CHECK: 999999
