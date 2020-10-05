org 0x7e00
jmp 0x0000:start

data:
	playerPosition: dw 0
    tablePosition: dw 0
    enemyPosition: times 5 dw 0
    bulletPosition: times 5 dw 0
    whichArray: db 0
    bulletOffset: dd 0
    counter: db 0
    bulletCounter: db 0
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
    mov word [playerPosition], 0x2000
    mov word [tablePosition], 1
    mov word [enemyPosition], 0
    mov byte[counter], 0
    call makegrid
    ;mov al, 51h
    ;call putchar
    
    
    ;Código do projeto...

makegrid:
    
    .waitForInput:
        ;colocando um limite para o uso do tiro, o jogador usa apenas 1 tiro a cada 4 ticks
        inc byte[bulletCounter]
        ;usando um ponteiro (bulletOffset) para guardar a posição de memória de bulletPosition que será usado depois para 
        ;ver em qual array do bulletPosition nós estamos checando.
        mov dword [bulletOffset], bulletPosition
        ;clear whichArray and bullet array
        mov byte[whichArray], 0
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
        cmp ah, 0x39
        je .spacePressed
        cmp ah, 0x1F
        je .sPressed
        cmp ah, 0x1E
        je .aPressed
        cmp ah, 0x20
        je .dPressed
        jmp .waitForInput
        .spacePressed:
            cmp byte[bulletCounter], 4
            jl .gridBegin
            mov byte[bulletCounter], 0
            mov si, enemyPosition
            call shootBullet
            jmp .gridBegin
        .sPressed:
            jmp .gridBegin
        .aPressed:
            ;compara se já esta no limite esquerdo.
            cmp word[playerPosition], 0x800
            je .gridBegin
            shr word [playerPosition], 1
            mov si, enemyPosition
            jmp .gridBegin
        .dPressed:
            ;compara se já esta no limite direito.
            cmp word[playerPosition], 0x8000
            je .gridBegin
            shl word [playerPosition], 1
            mov si, enemyPosition
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
    mov CX, 5
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

;essa função é o que faz os bits de todos os arrays do enemyPosition darem o shift left ao mesmo tempo.
enemyMovementChangeArray:

    mov ecx, 8
    REPEAT:
        shl word[enemyPosition + ecx], 5
        mov ax, 0x7C00
        and ax, word[enemyPosition + ecx - 2]
        shr ax, 10
        add word[enemyPosition + ecx], ax
        sub ecx, 2
        cmp ecx, 2
        jne REPEAT

    shl word[enemyPosition + 2], 5
    mov ax, 0x7C00
    and ax, word[enemyPosition]
    shr ax, 9
    add word[enemyPosition + 2], ax

    shl word[enemyPosition], 5

    call collisionBullet
    mov ecx, 0
    bulletMovementChangeArray:
        shr word[bulletPosition], 5
        mov ax, 0x1F
        and ax, word[bulletPosition + 2]
        shl ax, 10
        add word[bulletPosition], ax
        add ecx, 2
        
        shr word[bulletPosition + 2], 5
        mov ax, 0x1F
        and ax, word[bulletPosition + 4]
        shl ax, 10
        add word[bulletPosition + 2], ax
        add ecx, 2

        shr word[bulletPosition + 4], 5
        mov ax, 0x1F
        and ax, word[bulletPosition + 6]
        shl ax, 10
        add word[bulletPosition + 4], ax
        add ecx, 2

        shr word[bulletPosition + 6], 5
        mov ax, 0x1F
        and ax, word[bulletPosition + 8]
        shl ax, 10
        add word[bulletPosition + 6], ax
        add ecx, 2

        
        shr word[bulletPosition + 8], 5
        call collisionBullet
        ret


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
        add dword[bulletOffset], 2
        ;resetando table position para andar as 15 primeiras casas do próximo array.
        mov word[tablePosition], 1
        mov byte[counter], 0
        inc byte[whichArray]
        ret

collision:

    ;se o bit setado do playerPosition é igual ao bit setado do enemyPosition, houve colisão
    mov ax, word[playerPosition]
    and ax, word[enemyPosition + 8]
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
    ;se o array de inimigos que estamos checando não é o último array, pule para player não encontrado
    cmp byte[whichArray], 4
    jne playerNotEncountered
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
        je enemyNotEncountered
        jne enemyEncountered

        enemyEncountered:
        ;call collisionBullet
        mov ah, 0x0E
        mov al, 118
        int 10h
        ret

        enemyNotEncountered:
        mov ecx, [bulletOffset]
        mov ax, [ecx]
        and ax, word[tablePosition]
        cmp ax, 0
        je bulletNotEncountered
        jne bulletEncountered

            bulletEncountered:
                mov ah, 0x0E
                mov al, 124
                int 0x10
                ret

            bulletNotEncountered:
                mov ah, 0Eh
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
    mov ax, 0x1102
    mov bh, 0
    int 0x10
    ret

shootBullet:
    mov ax, word[playerPosition]
    shr ax, 5
    xor word[bulletPosition + 8], ax
    ;call printBullet
    ret

collisionBullet:
    mov ecx, 8
    compareArrays:  
        mov ax, [enemyPosition + ecx]
        and ax, [bulletPosition + ecx]
        cmp ax, 0
        jne killEnemy  
        je subtractCounter     
        killEnemy:
            xor [bulletPosition + ecx], ax
            xor [enemyPosition + ecx], ax
        subtractCounter:
            sub ecx, 2
            jnz compareArrays
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
