\ RUN: %{compile} %t && %t | FileCheck %s

: main

999999 888888 .S
. .
bye

;

\ CHECK: 999999 888888 888888 999999