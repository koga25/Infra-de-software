org 0x7e00
jmp 0x0000:start

data:
	playerPosition: dw 0
    tablePosition: dw 0
    enemyPosition: times 5 dw 0
    arrayLenght equ 2
    counter: db 0
    endGame: db "Game end", 0
    len equ endGame - $
	;Dados do projeto...

start:
    call changeFontSize
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov bl, 0
    mov bh, 0
    mov word [playerPosition], 0
    mov word [tablePosition], 1
    mov word [enemyPosition], 0
    mov byte[counter], 0
    call makegrid
    ;mov al, 51h
    ;call putchar
    
    
    ;Código do projeto...

makegrid:
    
    .waitForInput:
        call clear
        ;colocando a posição de memória de enemyPosition em si
        mov si, enemyPosition
        call enemyMovement
        ;int 16h com AH = 1 checa no buffer do teclado para ver se alguma tecla foi apertada, se
        ;o buffer estiver vazio ZF é setado, se não estiver vazio ZF é resetado.

        ;!* essa checagem não reseta o buffer, ou seja ao checar e verificar que uma tecla foi 
        ;apertada é necessario depois limpar ele.

        ;int 16h com AH = 0 pega o input não extendido e depois limpa o buffer.
        ;int 16h com AX = 0601h limpa todo o buffer.
        ;http://www.ctyme.com/intr/int-16.htm <- source
        mov ah, 1
        int 16h
        ;se ZF está setado não houve aperto de teclas, então pule para o desenho da grid.
        jz .gridBegin
        ;int 16h com AH = 0 espera por um input do teclado. o valor de AH recebido é comparado ao
        ;SCANCODE da tecla que foi apertada 
        ;https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
        xor ah, ah
        int 16h
        cmp ah, 0x11
        je .wPressed
        cmp ah, 0x1F
        je .sPressed
        cmp ah, 0x1E
        je .aPressed
        cmp ah, 0x20
        je .dPressed
        jmp .waitForInput
        .wPressed:
            jmp .gridBegin
        .sPressed:
            jmp .gridBegin
        .aPressed:
            ;compara se já esta no limite esquerdo.
            cmp word[playerPosition], 0x1000
            je .gridBegin
            shr word [playerPosition], 1
            mov si, enemyPosition
            mov cx, arrayLenght
            jmp .gridBegin
        .dPressed:
            ;compara se já esta no limite direito.
            cmp word[playerPosition], 0x4000
            je .gridBegin
            shl word [playerPosition], 1
            mov si, enemyPosition
            mov cx, arrayLenght
            jmp .gridBegin
            

    .gridBegin:
       call putchar
       ;checa se passou de 15 bits, se sim incremente SI para ir para a próxima posição do array
       call incrementCounter
       inc bl
       shl word[tablePosition], 1
       cmp  bl, 4
       jle .gridBegin
       call jumpLine
       mov bl, 0
       inc bh
       cmp bh, 14
       jle .gridBegin
       mov bh, 0
       mov word[tablePosition], 1
       call delay
       call resetCounters
       jmp .waitForInput

delay:
    ;faz o computador esperar o tempo determinado por CXDX, o intervalo de tempo utilizado está em microsegundos
    ;mov CX, 7
    mov AL, 0
    mov DX, 0x4120
    mov AH, 0x86
    int 15h
    ;http://www.ctyme.com/intr/int-15.htm <- source

clear:
    mov ah, 0
    mov al, 3
    int 10h
    ret

enemyMovement:
    ;movimento do inimigo é feito por operação bitwise, para andar para baixo temos que mover
    ;o bit setado pela mesma quantidade de colunas que a grid tem, nesse caso é 3.
    cmp word[si], 0 
    je .notSpawned
    jne .spawned
    .notSpawned:
        call .modulus
        ;div coloca o módulo da divisão no registro dl, por isso que antes de dividirmos nós zeramos dx
        mov word[enemyPosition], dx
        ret
    .spawned:
        ;shift left na segunda posição do array para andar para frente antes de colocar as últimas posições do primeiro
        ;array nos bits iniciais do segundo array.
        call enemyMovementChangeArray
        mov ax, 0x3F
        and ax, word[enemyPosition]
        cmp ax, 0
        je .addEnemies
        ret
        .addEnemies:
            call .modulus
            or word[enemyPosition], dx
            ret
    .modulus:
        ;interrupt 1Ah com AH = 0 pega o tempo do sistema pelo número de ticks do clock desde a meia noite, podemos usar 
        ;isso utilizar em conjunto com um and com um valor n²-1 para preencher um número random n para colocar os inimigos
        ;na tela.
        ;http://www.ctyme.com/intr/rb-2271.htm <- source
        mov AH, 0x00
        int 1Ah
        and dx, 31
        ret

enemyMovementChangeArray:

    shl word[enemyPosition + 8], 5
    mov ax, 0x7C00
    and ax, word[enemyPosition + 6]
    shr ax, 10
    add word[enemyPosition + 8], ax

    shl word[enemyPosition + 6], 5
    mov ax, 0x7C00
    and ax, word[enemyPosition + 4]
    shr ax, 10
    add word[enemyPosition + 6], ax

    shl word[enemyPosition + 4], 5
    mov ax, 0x7C00
    and ax, word[enemyPosition + 2]
    shr ax, 10
    add word[enemyPosition + 4], ax

    
    shl word[enemyPosition + 2], 5
    mov ax, 0x7C00
    and ax, word[enemyPosition]
    shr ax, 9
    add word[enemyPosition + 2], ax

    shl word[enemyPosition], 5
    ret

incrementCounter:
    inc byte[counter]
    cmp byte[counter], 0xF
    je .incrementSI
    ret
    .incrementSI:
        ;vá para a próxima posição do array, temos que adicionar a quantidade de bytes que contem em cada posição de array
        ;em SI
        add SI,2
        ;resetando table position para andar as 15 primeiras casas do próximo array.
        mov word[tablePosition], 1
        mov byte[counter], 0
        ret

collision:

    ;se o bit setado do playerPosition é igual ao bit setado do enemyPosition, houve colisão
    mov ax, word[playerPosition]
    and ax, word[enemyPosition]
    cmp ax, 0
    je notCollided
    jne collided
    notCollided:
        ret
    collided:
        call clear
        mov si, endGame
        call end1
        jmp $


putchar:
    ;se o bit setado do playerPosition é igual ao bit setado do tablePosition, coloque @ na tela
    mov ax, word[playerPosition]
    cmp ax, word[tablePosition]
    je playerEncountered
    jne playerNotEncountered

    ;se player não foi encontrado, faça o mesmo teste para inimigos
    playerNotEncountered:
        mov ax, word[si]
        and ax, word[tablePosition]
        cmp ax, 0
        mov ah, 0Eh
        je enemyNotEncountered
        jne enemyEncountered

        enemyEncountered:
        mov al, 118
        int 10h
        ret

        enemyNotEncountered:
        mov al, 46
        int 10h
        ret

    playerEncountered:
        call collision
        mov ah, 0x0E
        mov al, 64
        int 10h
        ret


jumpLine:
    mov ah, 0x0E
    mov al, 10
    int 10h
    mov al, 13
    int 10h
    ret

resetCounters:
    mov byte[counter], 0
    ret

changeFontSize:
    mov ax, 1104h
    mov bh, 0
    int 0x10
    ret

;colocando a mensagem na tela.
end1:
    lodsb
    mov ah, 0xE
    mov bh, 0
    mov bl, 0xF
    int 10h

    cmp al, 0
    jne end1
    ret
