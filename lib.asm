section .data
message: db  'hello, world!', 10
fileName: db 'It is very simple', 0
newline_char: db 10
codes: db '0123456789abcdef', 0
table: db '0123456789', 0
plusSign: db '+'
minusSign: db '-'
section .text
 
 
; Принимает код возврата и завершает текущий процесс
exit:
    mov     rax, 60; 
    xor     rdi, rdi
    syscall

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string_null_terminated:
    push rdi
    call string_length
    pop rsi
    mov rdx, rax
    mov rax, 1
    mov rdi, 1
    syscall
    ret

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
    xor rax, rax
.loop:
    cmp byte [rdi+rax], 0
    je .end
    inc rax
    jmp .loop
.end:
    ret

; Принимает код символа и выводит его в stdout
print_char:
    push rdi
    mov rdx, 1
    mov rax, 1
    mov rsi, rsp 
    mov rdi, 1
    syscall
    pop rdi
    ret 

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    mov rax, 1            ; 'write' syscall identifier
    mov rdi, 1            ; stdout file descriptor
    mov rsi, newline_char ; where do we take data from
    mov rdx, 1            ; the amount of bytes to write
    syscall
   ret

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
    push rcx
    xor rcx, rcx
    mov rax, rdi; get parametre from rdi
    mov rbx, 10
.save_loop:
    xor rdx, rdx
    div rbx
    push rdx
    inc rcx
    cmp rax, 0
    jne .save_loop
.print_loop:
    pop rax
    push rcx
    lea rsi, [table+rax]
    mov rax, 1
    mov rdi, 1
    mov rdx, 1
    syscall
    pop rcx
    dec rcx
    cmp rcx, 0
    jne .print_loop
    pop rcx
    ret

; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:
    cmp byte[rdi], '-'
    jz .printMinus
    cmp byte[rdi], '+'
    jz .printPlus
    call print_uint
    ret
.printMinus:
    push rdi
    mov rdx, 1
    mov rsi, minusSign
    mov rdi, 1
    mov rax, 1
    syscall
    pop rdi
    call print_uint
    ret
.printPlus:
    push rdi
    mov rdx, 1
    mov rsi, plusSign
    mov rdi, 1
    mov rax, 1
    syscall
    pop rdi
    xor rax, rax
    ret

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor rax, rax
    ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    mov rax, 0     
    mov rdi, 0     
    push 0         
    mov rsi, rsp  
    mov rdx, 1     
    syscall
    pop rax
    cmp rax, 10
    jz .A
    jmp .RET
.A:
    mov rax, 48
.RET:
    ret 

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    ret
 

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    xor r8, r8
    xor rcx, rcx
    mov r10, 10
.A:
    mov r8b, [rdi]
    inc rdi
    cmp r8b, '0'
    jb .NO
    cmp r8b, '9'
    ja .NO
    xor rax, rax
    sub r8b, '0'
    mov al, r8b
    inc rdx
.B:
    mov r8b, [rdi]
    inc rdi
    cmp r8b, '0'
    jb .OK
    cmp r8b, '9'
    ja .OK
    inc rdx
    mul r10
    sub r8b, '0'
    add rax, r8
    jmp .B
.OK:
.NO:
    mov rdx, rcx
    ret

; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    cmp byte[rdi], '-'
    jz .ne
    call parse_uint
    ret
.ne:
    inc rdi
    call parse_uint
    test rdx, rdx
    jz .er
    inc rdx
    neg rax
    ret
.er:
    xor rax, rax
    ret



; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    xor rax, rax
    ret