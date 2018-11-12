\ RUN: llforthc %s | lli | FileCheck %s

inbuf .
bye

\ CHECK: {{[0-9]+}}