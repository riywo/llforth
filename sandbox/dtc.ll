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
  %start = getelementptr [5 x i8*], [5 x i8*]* @code, i32 0 , i32 0
  br label %top
top:
  %ip = phi i8** [%start, %entry], [%new_ip, %next]
  %addr = load i8*, i8** %ip
  %new_ip = getelementptr i8*, i8** %ip, i32 1
  indirectbr i8* %addr, [label %A, label %B, label %exit]
next:
  br label %top

A:
  call i32 @putchar(i32 65)
  br label %next
B:
  call i32 @putchar(i32 66)
  br label %next
exit:
  call i32 @putchar(i32 10)
  ret i32 0
}
