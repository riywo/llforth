declare i32* @gets(i32* nocapture) nounwind
declare i32 @putchar(i32)

@sp = private global i32 undef
@stack = private global [10 x i32] undef

define void @push(i32 %val) {
  %sp = load i32, i32* @sp
  %addr = getelementptr [10 x i32], [10 x i32]* @stack, i32 0, i32 %sp
  store i32 %val, i32* %addr
  %newsp = add i32 %sp, 1
  store i32 %newsp, i32* @sp
  ret void
}

define i32 @peek() {
  %sp = load i32, i32* @sp
  %topsp = sub i32 %sp, 1
  %addr = getelementptr [10 x i32], [10 x i32]* @stack, i32 0, i32 %topsp
  %val = load i32, i32* %addr
  ret i32 %val
}

define i32 @pop() {
  %val = call i32 @peek()
  %sp = load i32, i32* @sp
  %newsp = sub i32 %sp, 1
  store i32 %newsp, i32* @sp
  ret i32 %val
}

define void @push_input() {
  %buf = alloca [10 x i32]
  %buf.ptr = getelementptr [10 x i32], [10 x i32]* %buf, i32 0, i32 0
  %ptr = call i32* @gets(i32* %buf.ptr)
  %input = load i32, i32* %ptr
  call void @push(i32 %input)
  ret void
}

define void @pop_print() {
  %val = call i32 @pop()
  call i32 @putchar(i32 %val)
  ret void
}

define i32 @main() {
  call void @push_input()
  call void @push_input()
  call void @push_input()
  call void @push_input()
  call void @pop_print()
  call void @pop_print()
  call void @pop_print()
  call void @pop_print()
  ret i32 0
}
