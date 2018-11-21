\ RUN: llforthc %s | %{lli} | FileCheck %s

999 999000 + .
1000000 1 - .
3 333333 * .
1999998 2 / .

bye

\ CHECK: 999999 999999 999999 999999