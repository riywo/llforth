\ RUN: llforthc %s > %t && echo '999999' | lli %t | FileCheck %s

inbuf word
inbuf number
.
bye

\ CHECK: 999999
