section .data

newline_char: db 10
table: db '0123456789', 0

section .text
 
; Принимает код возврата и завершает текущий процесс
exit:
    mov     rax, 60; 
    xor     rdi, rdi
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
    cmp rdi, 0
    jge print_uint
    neg rdi
    push rdi
    mov edi, '-'
    call print_char
    pop rdi
    jmp print_uint


; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor r8, r8
    xor r9, r9

    .string_equals_loop:
        mov r8b, byte [rdi]
        mov r9b, byte [rsi]
        cmp r8, r9
        jnz .string_equals_end_0
        inc rdi
        inc rsi
        cmp r8, 0
        jnz .string_equals_loop
        mov rax, 1
        ret
        
    .string_equals_end_0:
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
    push r12
	mov r9,rdi	; pointer
	mov r10,rsi ; size
	xor r11,r11	; length
	mov r12,rdi
	
    .tabs:
		push r11
		call read_char
		pop r11
		cmp rax,0x0
		je .end
		cmp rax,0x20
		je .tabs
		cmp rax,0x9
		je .tabs
		cmp rax,0xA
		je .tabs
		mov [r9],rax
		inc r9
		inc r11
		dec r10
	
    .reader:
		cmp r10,0
		je .nul
		push r11
		call read_char
		pop r11
		cmp rax,0x0
		je .end
		cmp rax,0x20
		je .end
		cmp rax,0x9
		je .end
		cmp rax,0xA
		je .end
		mov [r9],rax
		inc r9
		inc r11
		dec r10
		jmp .reader
		
    .nul:
		mov rdx,0
		pop r12
		mov rax,0
		ret

    .end:
		mov rdx,r11
		mov rax,r12
		mov byte [r9],0
		pop r12
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
        jb .end
        cmp r8b, '9'
        ja .end
        xor rax, rax
        sub r8b, '0'
        mov al, r8b
        inc rcx

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
        inc rdx
        neg rax
        ret

    .err:
        xor rax, rax
        ret

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    push rdi
	push rsi
	push rdx
	call string_length
	cmp rdx, rax
	jl .not_eq

    .loop:	
        mov rdx, [rdi]
        mov [rsi], rdx
        cmp byte[rdi], 0
        je .end
        inc rdi
        inc rsi
        jmp .loop

    .not_eq:
        xor rax, rax

    .end:
        pop rdx
        pop rsi
        pop rdi
        ret