\ RUN: llforthc %s > %t && echo 'w1' | %{lli} %t | FileCheck %s

inbuf word
.
inbuf prints
bye

\ CHECK: 2 w1