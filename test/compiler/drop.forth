\ RUN: llforthc %s | lli | FileCheck %s

999999 888888 drop .
bye

\ CHECK: 999999