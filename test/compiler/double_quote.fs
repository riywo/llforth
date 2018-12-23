\ RUN: %{compile} %t && %t | FileCheck %s

: main

." Hello world!"
bye

;

\ CHECK: Hello world!