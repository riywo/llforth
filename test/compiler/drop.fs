\ RUN: %{compile} %t && %t | FileCheck %s

: main

999999 888888 drop .
bye

;

\ CHECK: 999999