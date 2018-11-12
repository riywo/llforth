\ RUN: llforthc %s > %t && echo '.' | lli %t | FileCheck %s

9999999
inbuf word
.
inbuf find
execute
bye

\ CHECK: 1 9999999
