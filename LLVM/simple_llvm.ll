@.str = constant [14 x i8] c"Hello, LLVM!\0A\00"

define i32 @main() {
  %1 = alloca i32
  store i32 0, i32* %1
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr ([14 x i8], [14 x i8]* @.str, i32 0, i32 0))
  ret i32 0
}

declare i32 @printf(i8*, ...)
