section .text
 
; Принимает код возврата и завершает текущий процесс
exit:
    mov rax, 60
    syscall

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
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
        cmp byte[rdi+rax], 0
        je .return
        inc rax
        jmp .loop

    .return:
        ret

; Переводит строку (выводит символ с кодом 0xA)
; После будет тело функции print_char, хотя с jump это более интуитивно понятно
print_newline:
    mov rdi, 0xA
    jmp print_char

; Принимает код символа и выводит его в stdout
print_char:
    push rdi
    mov rdx, 1              ; the amount of bytes to write
    mov rax, 1              ; 'write' syscall identifier
    mov rsi, rsp            ; where do we take data from
    mov rdi, 1              ; stdout file descriptor
    syscall
    pop rdi
    ret

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
    mov r8, rsp             ; r8 = return address
    push 0                  ; null-terminate
    mov rax, rdi
    mov rcx, 10
    
    .loop:
        xor rdx, rdx
        div rcx             ; в rax оставшееся число, в rdx остаток от деления (текущая цифра)
        add rdx, 0x30
        dec rsp
        mov [rsp], dl       ; младший байт rdx
        test rax, rax
        jnz .loop
        mov rdi, rsp
        call print_string
        mov rsp, r8
        ret
    
; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:
    test rdi, rdi
    jns print_uint
    neg rdi
    push rdi
    mov rdi, '-'
    call print_char
    pop rdi
    jmp print_uint


; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor r8, r8
    xor r9, r9
    xor rax, rax

    .string_equals_loop:
        mov r8b, [rdi + rax]
        mov r9b, [rsi + rax]
        cmp r8b, r9b
        jnz .return_0
        inc rax
        cmp r8, 0
        jnz .string_equals_loop
        mov rax, 1
        ret
        
    .return_0:
        xor rax, rax
        ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    xor rax, rax
    xor rdi, rdi
    push 0         
    mov rsi, rsp  
    mov rdx, 1
    syscall
    pop rax
    cmp rax, 0x0
    jnz .return
    mov rax, 0

    .return:
		ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    push r12                ; callee-saved register
	mov r9, rdi	            ; pointer where is the next character written to
	xor r11, r11	        ; index (at the end is the length)
	mov r12, rdi
	
    .read_symbol:
        push r11            ; caller-saved, syscall changes the value
		push rsi
		call read_char
		pop rsi
		pop r11
		cmp al, 0x0        ; null
        je .end
		cmp al, 0x20       ; пробел
        je .maybe_end
		cmp al, 0x9        ; табуляция
		je .maybe_end
		cmp al, 0xA        ; перевод строки
		je .maybe_end
		mov [r9], al
		inc r9              ; next symbol
		inc r11             ; index/length ++
		cmp r11, rsi        ; if (в буфере больше нет свободного места), то то выдаем ошибку переполнения в виде return 0
		jge .return_0
		jmp .read_symbol
		
    .return_0:
		mov rdx, 0
		pop r12
		mov rax, 0
		ret

    .maybe_end:
        cmp r11, 0x0
        je .read_symbol

    .end:
		mov byte[r9], 0
		mov rdx, r11
		mov rax, r12
		pop r12
		ret

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    xor r8, r8              ; текущий символ
    xor rcx, rcx            ; счетчик длины
    mov r10, 10
    xor rax, rax            ; результат

    .B:
        mov r8b, [rdi]
        inc rdi
        cmp r8b, '0'
        jb .end
        cmp r8b, '9'
        ja .end
        inc rcx
        mul r10
        sub r8b, '0'
        add rax, r8
        jmp .B

    .end:
        mov rdx, rcx
        ret


; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    cmp byte[rdi], '-'
    jz .negative
    call parse_uint
    ret

    .negative:
        inc rdi
        call parse_uint
        test rdx, rdx
        jz .err
        inc rdx             ; включаем в длину минус
        neg rax
        ret

    .err:
        xor rax, rax
        ret

; Принимает указатель на строку (rdi), указатель на буфер (rsi) и длину буфера (rdx)
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    call string_length
    cmp rax, rdx
    jg .return_0
    mov rcx, rax
    mov r8, rsi
    inc rcx        ; Very important!!!
    mov rsi, rdi   ; Адрес области "откуда"
    mov rdi, r8    ; Адрес области "куда"
    rep movsb
    ret

    .return_0:
        xor rax, rax
        ret
