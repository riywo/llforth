\ RUN: llforthc %s | lli | FileCheck %s

999999 888888 777777 .S
bye

\ CHECK: 777777 888888 999999