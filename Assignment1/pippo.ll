define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) #0 {

  %3 = add nsw i32 %1, 5      ; Assegno un valore ad una variabile
  %4 = add nsw i32 %3, 1      ; Questa è la a=b+1
  %5 = sub nsw i32 %4, 1      ; Questa è la c=a-1
                              ; Converrebbe fare quindi C = B
  ret i32 %4
}
