declare i32 @putchar(i32)

@xt_A = private constant i8* blockaddress(@main, %i_A)
@xt_B = private constant i8* blockaddress(@main, %i_B)
@xt_exit = private constant i8* blockaddress(@main, %i_exit)

@code = private constant [5 x i8**] [
  i8** @xt_A,
  i8** @xt_B,
  i8** @xt_A,
  i8** @xt_B,
  i8** @xt_exit
]

define i32 @main() {
entry:
  %start = getelementptr [5 x i8**], [5 x i8**]* @code, i32 0 , i32 0
  br label %top
top:
  %ip = phi i8*** [%start, %entry], [%new_ip, %next]
  %w = load i8**, i8*** %ip
  %addr = load i8*, i8** %w
  %new_ip = getelementptr i8**, i8*** %ip, i32 1
  indirectbr i8* %addr, [label %i_A, label %i_B, label %i_exit]
next:
  br label %top

i_A:
  call i32 @putchar(i32 65)
  br label %next
i_B:
  call i32 @putchar(i32 66)
  br label %next
i_exit:
  call i32 @putchar(i32 10)
  ret i32 0
}
