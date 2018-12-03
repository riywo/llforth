\ RUN: %{compile} %t && %t | FileCheck %s

: main

999999 dup . .
bye

;

\ CHECK: 999999 999999