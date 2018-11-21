\ RUN: %{compile} %t && %t | FileCheck %s

1
branch0 .end
999999 .

0
branch0 .end
777777 .

.end:
888888 .
bye

\ CHECK: 999999 888888