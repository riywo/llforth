\ RUN: %{compile} %t && %t | FileCheck %s

: main

1
0branch .end
999999 .

0
0branch .end
777777 .

.end:
888888 .
bye

;

\ CHECK: 999999 888888