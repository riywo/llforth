declare i32 @putchar(i32)
declare i8* @gets(i8* nocapture) nounwind
declare i32 @strcmp(i8*, i8*) nounwind

%node = type{ %node*, i8*, i32 }

@foo.key = private constant [4 x i8] c"foo\00"
@foo = private constant %node {
  %node* @bar,
  i8* getelementptr([4 x i8], [4 x i8]* @foo.key, i32 0, i32 0),
  i32 1
}

@bar.key = private constant [4 x i8] c"bar\00"
@bar = private constant %node {
  %node* @baz,
  i8* getelementptr([4 x i8], [4 x i8]* @bar.key, i32 0, i32 0),
  i32 2
}

@baz.key = private constant [4 x i8] c"baz\00"
@baz = private constant %node {
  %node* null,
  i8* getelementptr([4 x i8], [4 x i8]* @baz.key, i32 0, i32 0),
  i32 3
}

define i8* @get_key(%node* %n) {
  %ptr = getelementptr %node, %node* %n, i32 0, i32 1
  %val = load i8*, i8** %ptr
  ret i8* %val
}

define i32 @get_value(%node* %n) {
  %ptr = getelementptr %node, %node* %n, i32 0, i32 2
  %val = load i32, i32* %ptr
  ret i32 %val
}

define %node* @get_next(%node* %n) {
  %ptr = getelementptr %node, %node* %n, i32 0, i32 0
  %addr = load %node*, %node** %ptr
  ret %node* %addr
}

define %node* @find_node(%node* %begin_node, i8* %find_key) {
entry:
  br label %loop
loop:
  %current_node = phi %node* [%begin_node, %entry], [%next_node, %next]
  %key = call i8* @get_key(%node* %current_node)
  %is_key.32 = call i32 @strcmp(i8* %key, i8* %find_key)
  %is_key = icmp eq i32 %is_key.32, 0
  br i1 %is_key, label %return, label %next
next:
  %next_node = call %node* @get_next(%node* %current_node)
  %is_end = icmp eq %node* %next_node, null
  br i1 %is_end, label %return_null, label %loop
return:
  ret %node* %current_node
return_null:
  ret %node* null
}

define i32 @main() {
entry:
  %buf = alloca [10 x i8]
  %buf.ptr = getelementptr [10 x i8], [10 x i8]* %buf, i32 0, i32 0
  call i8* @gets(i8* %buf.ptr)
  %find_node = call %node* @find_node(%node* @foo, i8* %buf.ptr)
  %is_null = icmp eq %node* %find_node, null
  br i1 %is_null, label %not_found, label %found
found:
  %val = call i32 @get_value(%node* %find_node)
  ret i32 %val
not_found:
  ret i32 255
}
