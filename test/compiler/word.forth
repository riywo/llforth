\ RUN: %{compile} %t && echo 'w1' | %t | FileCheck %s

inbuf word
.
inbuf prints
bye

\ CHECK: 2 w1