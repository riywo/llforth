declare i32 @putchar(i32)

@code = private constant [5 x i8*] [
  i8* blockaddress(@main, %A),
  i8* blockaddress(@main, %B),
  i8* blockaddress(@main, %A),
  i8* blockaddress(@main, %B),
  i8* blockaddress(@main, %exit)
]

define i32 @main() {
entry:
  br label %top
top:
  %pc = phi i32 [0, %entry], [%new_pc, %next]
  %ptr = getelementptr [5 x i8*], [5 x i8*]* @code, i32 0, i32 %pc
  %addr = load i8*, i8** %ptr
  indirectbr i8* %addr, [label %A, label %B, label %exit]
next:
  %new_pc = add i32 %pc, 1
  br label %top

A:
  call i32 @putchar(i32 65)
  br label %next
B:
  call i32 @putchar(i32 66)
  br label %next
exit:
  ret i32 0
}
