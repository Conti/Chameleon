; Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
;
; @APPLE_LICENSE_HEADER_START@
; 
; Portions Copyright (c) 2003 Apple Computer, Inc.  All Rights
; Reserved.  This file contains Original Code and/or Modifications of
; Original Code as defined in and that are subject to the Apple Public
; Source License Version 2.0 (the "License").  You may not use this file
; except in compliance with the License.  Please obtain a copy of the
; License at http://www.apple.com/publicsource and read it before using
; this file.
; 
; The Original Code and all software distributed under the License are
; distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
; EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
; INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
; License for the specific language governing rights and limitations
; under the License.
; 
; @APPLE_LICENSE_HEADER_END@

;
; This version of cdboot loads the MBR of bios drive 0x80.
;
; Modifications by Tam�s Kos�rszky on 2009-03-30
;

;
; Set to 1 to enable obscure debug messages.
;
DEBUG				EQU		0

;
; Set to 1 to enable unused code.
;
UNUSED				EQU		0

;
; Set to 1 to enable verbose mode.
;
VERBOSE				EQU		1

;
; Various constants.
;
NULL		   		EQU		0
CR					EQU		0x0D
LF					EQU		0x0A

;
; Macros.
;
%macro jmpabs 1
	push	WORD %1
	ret
%endmacro

%macro DebugCharMacro 1
	pushad
	mov		al, %1
	call	print_char
	call	getc
	popad
%endmacro

%macro DebugPauseMacro 0
    push	ax
    call	getc
    pop		ax
%endmacro

%macro PrintCharMacro 1
	pushad
	mov		al, %1
	call	print_char
	popad
%endmacro

%macro PutCharMacro 1
	call	print_char
%endmacro

%macro PrintHexMacro 1
	call	print_hex
%endmacro

%macro PrintString 1
	mov		si, %1
	call	print_string
%endmacro
        
%macro LogString 1
	mov		di, %1
	call	log_string
%endmacro

%if DEBUG
  %define DebugChar(x) DebugCharMacro x
  %define DebugPause(x)  DebugPauseMacro
  %define PrintChar(x) PrintCharMacro x
  %define PutChar(x) PutCharMacro
  %define PrintHex(x) PrintHexMacro x
%else
  %define DebugChar(x)
  %define DebugPause(x)
  %define PrintChar(x)
  %define PutChar(x)
  %define PrintHex(x)
%endif

kBoot0Segment		  EQU  0x0000
kBoot0Stack       EQU  0xFFF0     ; boot0 stack pointer
kBoot0LoadAddr    EQU  0x7C00			; boot0 load address
kBoot0RelocAddr   EQU  0xE000     ; boot0 relocated address
kSectorBytes      EQU  2048 			; sector size in bytes
maxSectorCount	  EQU  32		  		; maximum sector count for readSectors

;--------------------------------------------------------------------------
; Start of text segment.

    SEGMENT .text

    ORG     kBoot0RelocAddr
    
;--------------------------------------------------------------------------
; Boot code is loaded at 0:7C00h.
;

start:
	cli
	jmp			start1
	times		8-($-$$) nop	; Put boot information table at offset 8
	    
; El Torito boot information table, filled in by the
; mkisofs -boot-info-table option, if used.
bi_pvd:         dd 0			; LBA of primary volume descriptor
bi_file:        dd 0			; LBA of boot file
bi_length:		dd 0			; Length of boot file
bi_csum:		dd 0			; Checksum of boot file
bi_reserved:	times 10 dd 0	; Reserved

;--------------------------------------------------------------------------

start1:
    ;
    ; Set up the stack to grow down from kBoot0Segment:kBoot0Stack.
    ; Interrupts should be off while the stack is being manipulated.
    ;
    cli                             ; interrupts off
    xor     ax, ax                  ; zero ax
    mov     ss, ax                  ; ss <- 0
    mov     sp, kBoot0Stack         ; sp <- top of stack
    sti                             ; reenable interrupts

    mov     es, ax                  ; es <- 0
    mov     ds, ax                  ; ds <- 0

    ;
    ; Relocate boot0 code.
    ;
    mov     si, kBoot0LoadAddr      ; si <- source
    mov     di, kBoot0RelocAddr     ; di <- destination
    cld                             ; auto-increment SI and/or DI registers
    mov     cx, kSectorBytes/2      ; copy 256 words
    repnz   movsw                   ; repeat string move (word) operation

    ;
    ; Code relocated, jump to start_reloc in relocated location.
    ;
    jmp     kBoot0Segment:start_reloc

;--------------------------------------------------------------------------
; Start execution from the relocated location.
;
start_reloc:

%if VERBOSE
    LogString(init_str)
%endif

%if VERBOSE
    LogString(read_str)
%endif

    ; forcing BIOS Int 13h operations to use drive 0x80
    mov     WORD [gBIOSDriveNumber], 0x80			; save BIOS drive number
    mov     al, 1
    mov     edx, kBoot0LoadAddr           ; load MBR code to 0x7C00
    mov     ecx, 0                        ; load sector 0 of drive 0x80
    call    readLBA
    
%if VERBOSE
    LogString(done_str)
%endif

%if UNUSED
    LogString(keypress_str)
    call	getc
%endif

  	; Jump to newly-loaded booter
    mov     dl, [gBIOSDriveNumber]			; load BIOS drive number
    jmp     kBoot0Segment:kBoot0LoadAddr

error:

%if VERBOSE
    LogString(error_str)
%endif

.loop:
    hlt
	jmp		.loop
    
;; 
;; Support functions
;;

;--------------------------------------------------------------------------
; readSectors - Reads more than 127 sectors using LBA addressing.
;
; Arguments:
;   AX = number of 2048-byte sectors to read (valid from 1-320).
;   EDX = pointer to where the sectors should be stored.
;   ECX = sector offset in partition 
;
; Returns:
;   CF = 0  success
;        1 error
;
readSectors:
	pushad
	mov		bx, ax

.loop:
	mov		al, '.'
	call	print_char
	xor		eax, eax						; EAX = 0
	mov		al, bl							; assume we reached the last block.
	cmp		bx, maxSectorCount				; check if we really reached the last block
	jb		.readBlock						; yes, BX < MaxSectorCount
	mov		al, maxSectorCount				; no, read MaxSectorCount

.readBlock:
	call	readLBA
	jc		.exit
	sub		bx, ax							; decrease remaning sectors with the read amount
	jz		.exit							; exit if no more sectors left to be loaded
	add		ecx, eax						; adjust LBA sector offset
	shl		eax, 11							; convert CD sectors to bytes
	add		edx, eax						; adjust target memory location
	jmp		.loop							; read remaining sectors

.exit:
	popad
	ret

;--------------------------------------------------------------------------
; readLBA - Read sectors from a partition using LBA addressing.
;
; Arguments:
;   AL = number of 512-byte sectors to read (valid from 1-127).
;   EDX = pointer to where the sectors should be stored.
;   ECX = sector offset in partition 
;   [gBIOSDriveNumber] = drive number (0x80 + unit number)
;
; Returns:
;   CF = 0  success
;        1 error
;
readLBA:
    pushad                          		; save all registers
    push    es								; save ES
    mov     bp, sp                 			; save current SP

    ;
    ; Convert EDX to segment:offset model and set ES:BX
    ;
    ; Some BIOSes do not like offset to be negative while reading
    ; from hard drives. This usually leads to "boot1: error" when trying
    ; to boot from hard drive, while booting normally from USB flash.
    ; The routines, responsible for this are apparently different.
    ; Thus we split linear address slightly differently for these
    ; capricious BIOSes to make sure offset is always positive.
    ;

	mov		bx, dx							; save offset to BX
	and		bh, 0x0f						; keep low 12 bits
	shr		edx, 4							; adjust linear address to segment base
	xor		dl, dl							; mask low 8 bits
	mov		es, dx							; save segment to ES

    ;
    ; Create the Disk Address Packet structure for the
    ; INT13/F42 (Extended Read Sectors) on the stack.
    ;

    ; push    DWORD 0              			; offset 12, upper 32-bit LBA
    push    ds                      		; For sake of saving memory,
    push    ds                      		; push DS register, which is 0.

    push    ecx

    push    es                      		; offset 6, memory segment

    push    bx                      		; offset 4, memory offset

    xor     ah, ah             				; offset 3, must be 0
    push    ax                      		; offset 2, number of sectors

    push    WORD 16                 		; offset 0-1, packet size

    ;
    ; INT13 Func 42 - Extended Read Sectors
    ;
    ; Arguments:
    ;   AH    = 0x42
    ;   [gBIOSDriveNumber] = drive number (0x80 + unit number)
    ;   DS:SI = pointer to Disk Address Packet
    ;
    ; Returns:
    ;   AH    = return status (success is 0)
    ;   carry = 0 success
    ;           1 error
    ;
    ; Packet offset 2 indicates the number of sectors read
    ; successfully.
    ;
	mov     dl, [gBIOSDriveNumber]			; load BIOS drive number
	mov     si, sp
	mov     ah, 0x42
	int     0x13

	jnc     .exit

	DebugChar('R')                  ; indicate INT13/F42 error

    ;
    ; Issue a disk reset on error.
    ; Should this be changed to Func 0xD to skip the diskette controller
    ; reset?
    ;

%if VERBOSE
    LogString(readerror_str)
	mov		eax, ecx
	call	print_hex
%endif

	xor     ax, ax                  		; Func 0
	int     0x13                    		; INT 13
	stc                             		; set carry to indicate error

.exit:
	mov     sp, bp                  		; restore SP
    pop     es								; restore ES
    popad
    ret
	
;--------------------------------------------------------------------------
; Write a string with 'cdboot: ' prefix to the console.
;
; Arguments:
;   ES:DI   pointer to a NULL terminated string.
;
; Clobber list:
;   DI
;
log_string:
    pushad

    push	di
    mov		si, log_title_str
    call	print_string

    pop		si
    call	print_string

    popad
    
    ret

;-------------------------------------------------------------------------
; Write a string to the console.
;
; Arguments:
;   DS:SI   pointer to a NULL terminated string.
;
; Clobber list:
;   AX, BX, SI
;
print_string:
    mov     bx, 1                   		; BH=0, BL=1 (blue)

.loop:
    lodsb                           		; load a byte from DS:SI into AL
    cmp     al, 0               			; Is it a NULL?
    je      .exit                   		; yes, all done
    mov     ah, 0xE                 		; INT10 Func 0xE
    int     0x10                    		; display byte in tty mode
    jmp     .loop

.exit:
    ret

;%if DEBUG

;--------------------------------------------------------------------------
; Write the 4-byte value to the console in hex.
;
; Arguments:
;   EAX = Value to be displayed in hex.
;
print_hex:
    pushad
    mov     cx, WORD 4
    bswap   eax
.loop:
    push    ax
    ror     al, 4
    call    print_nibble            		; display upper nibble
    pop     ax
    call    print_nibble            		; display lower nibble
    ror     eax, 8
    loop    .loop

%if UNUSED
	mov     al, 10							; carriage return
	call    print_char
	mov     al, 13
	call    print_char
%endif ; UNUSED

    popad
    ret
	
print_nibble:
    and     al, 0x0f
    add     al, '0'
    cmp     al, '9'
    jna     .print_ascii
    add     al, 'A' - '9' - 1
.print_ascii:
    call    print_char
    ret

;--------------------------------------------------------------------------
; getc - wait for a key press
;
getc:
    pushad
    mov     ah, 0
    int		0x16
    popad
    ret

;--------------------------------------------------------------------------
; Write a ASCII character to the console.
;
; Arguments:
;   AL = ASCII character.
;
print_char:
    pushad
    mov     bx, 1                   		; BH=0, BL=1 (blue)
    mov     ah, 0x0e                		; bios INT 10, Function 0xE
    int     0x10                    		; display byte in tty mode
    popad
    ret

;--------------------------------------------------------------------------
; Static data.
;

%if VERBOSE
log_title_str   db		CR, LF, 'cdboot: ', NULL
init_str        db		'init', NULL
read_str        db		'reading MBR from device 0x80', NULL
done_str        db		'done', NULL
readerror_str		db		'BIOS disk read error at sector: ', NULL
error_str       db		'error', NULL
%endif

%if UNUSED
keypress_str		db		'Press any key to continue...', NULL
%endif


;; Pad this file to a size of 2048 bytes (one CD sector).
pad:
        times 2048-($-$$) db 0

;
; Global variables
;

	ABSOLUTE		kBoot0RelocAddr + kSectorBytes

gBIOSDriveNumber	resw	1

;   END
