%include "nasmhead.inc"

%define PWM_BUF_SIZE 4

section .data align=64

	DECL PWM_FULL_TAB

%if PWM_BUF_SIZE = 8
	db	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
	db	0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00
	db	0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40
%else
	db	0x40, 0x00, 0x00, 0x80
	db	0x80, 0x40, 0x00, 0x00
	db	0x00, 0x80, 0x40, 0x00
	db	0x00, 0x00, 0x80, 0x40
%endif


section .bss align=64


	extern M_SH2
	extern S_SH2
	extern _32X_MINT
	extern _32X_SINT


	DECL PWM_FIFO_R
	resw PWM_BUF_SIZE

	DECL PWM_FIFO_L
	resw PWM_BUF_SIZE

	DECL PWM_RP_R
	resd 1

	DECL PWM_WP_R
	resd 1

	DECL PWM_RP_L
	resd 1

	DECL PWM_WP_L
	resd 1

	DECL PWM_Cycle
	resd 1

	DECL PWM_Cycle_Cnt
	resd 1

	DECL PWM_Int_Cnt
	resd 1

	DECL PWM_Out_R
	resd 1

	DECL PWM_Out_L
	resd 1

	DECL PWM_Mode
	resd 1

	DECL PWM_Cycles
	resd 1

	DECL PWM_Int
	resd 1

	DECL PWM_Enable
	resd 1

	DECL PWM_Cycle_Tmp
	resd 1

	DECL PWM_Int_Tmp
	resd 1

	DECL PWM_FIFO_L_Tmp
	resd 1

	DECL PWM_FIFO_R_Tmp
	resd 1



section .text align=64


	extern SH2_Interrupt


	; void PWM_Init(void)
	DECLF PWM_Init, 0

		xor eax, eax
		mov [PWM_Mode], eax
		mov [PWM_Out_R], eax
		mov [PWM_Out_L], eax
		mov [PWM_FIFO_R + 0x0], eax
		mov [PWM_FIFO_R + 0x4], eax
		mov [PWM_FIFO_R + 0x8], eax
		mov [PWM_FIFO_R + 0xC], eax
		mov [PWM_FIFO_L + 0x0], eax
		mov [PWM_FIFO_L + 0x4], eax
		mov [PWM_FIFO_L + 0x8], eax
		mov [PWM_FIFO_L + 0xC], eax
		mov [PWM_RP_R], eax
		mov [PWM_WP_R], eax
		mov [PWM_RP_L], eax
		mov [PWM_WP_L], eax
		mov [PWM_Cycle_Tmp], eax
		mov [PWM_Int_Tmp], eax
		mov [PWM_FIFO_L_Tmp], eax
		mov [PWM_FIFO_R_Tmp], eax
		xor ecx, ecx
		call PWM_Set_Cycle
		xor ecx, ecx
		call PWM_Set_Int
		ret


	ALIGN32

	; void PWM_Set_Cycle(unsigned int cycle)
	DECLF PWM_Set_Cycle, 4

		mov [PWM_Cycle_Tmp], ecx
		dec ecx
		and ecx, 0xFFF
		mov [PWM_Cycle], ecx
		mov ecx, [PWM_Cycles]
		mov [PWM_Cycle_Cnt], ecx
		ret


	ALIGN32

	; void PWM_Set_Int(unsigned int int_time)
	DECLF PWM_Set_Int, 4

		and cl, 0xF
		jz short .spec

		mov [PWM_Int], cl
		mov [PWM_Int_Cnt], cl
		ret

	.spec
		mov byte [PWM_Int], 16
		mov byte [PWM_Int_Cnt], 16
		ret


	ALIGN32

	; void PWM_Clear_Timer(void)
	DECLF PWM_Clear_Timer, 0
		mov dword [PWM_Cycle_Cnt], 0
		ret


	ALIGN32

	; void PWM_Update_Timer(unsigned int cycle)
	DECLF PWM_Update_Timer, 4

		push ebx
		push edx

		mov bl, [PWM_Mode]
		mov bh, [PWM_Enable]
		mov eax, [PWM_Cycle]
		and bx, 0xFF0F
		jz short .stopped

		test eax, eax
		mov edx, [PWM_Cycle_Cnt]
		jz short .stopped

	.loop
		cmp edx, ecx
		mov bl, [PWM_Int_Cnt]
		jbe short .counter_raised
		
	.stopped
		pop edx
		pop ebx
		ret
		
	ALIGN4
	
	.counter_raised
		call PWM_Shift_Data

		dec bl
		jz short .do_int

		add edx, eax
;		cmp edx, ecx
;		jbe short .loop

		mov [PWM_Int_Cnt], bl
		mov [PWM_Cycle_Cnt], edx

		pop edx
		pop ebx
		ret

	ALIGN4

	.do_int
		add edx, eax
		mov bl, [PWM_Int]
		mov [PWM_Cycle_Cnt], edx
		mov [PWM_Int_Cnt], bl

		mov al, [_32X_MINT]
		mov bl, [_32X_SINT]
		test al, 1
		jz short .no_mint

		mov ecx, M_SH2
		mov dl, 6
		call SH2_Interrupt

	.no_mint
		test bl, 1
		jz short .no_sint

		mov ecx, S_SH2
		mov dl, 6
		call SH2_Interrupt

	.no_sint
		pop edx
		pop ebx
		ret


	ALIGN32

	PWM_Shift_Data

		push eax
		push edx

		mov eax, [PWM_Mode]
		and eax, 0xF
		jmp [.Table + eax * 4]

	ALIGN4
	
	.Table
		dd .Rx_Lx, .Rx_LL, .Rx_LR, .Rx_Lx
		dd .RL_Lx, .RR_LL, .RL_LR, .RL_Lx
		dd .RR_Lx, .RR_LL, .RL_LR, .RR_Lx
		dd .Rx_Lx, .Rx_LL, .Rx_LR, .Rx_Lx

	ALIGN32

	.Rx_LL
		mov edx, [PWM_RP_L]
		cmp edx, [PWM_WP_L]
		je short .Rx_LL_Empty

		mov ax, [PWM_FIFO_L + edx * 2]
		inc edx
		mov [PWM_Out_L], ax
		and edx, byte (PWM_BUF_SIZE - 1)
		mov [PWM_RP_L], edx
		pop edx
		pop eax
		ret

	ALIGN4

	.Rx_LL_Empty
		pop edx
		pop eax
		ret


	ALIGN32

	.Rx_LR
		mov edx, [PWM_RP_L]
		cmp edx, [PWM_WP_L]
		je short .Rx_LR_Empty

		mov ax, [PWM_FIFO_L + edx * 2]
		inc edx
		mov [PWM_Out_R], ax
		and edx, byte (PWM_BUF_SIZE - 1)
		mov [PWM_RP_L], edx
		pop edx
		pop eax
		ret

	ALIGN4

	.Rx_LR_Empty
		pop edx
		pop eax
		ret


	ALIGN32

	.RL_Lx
		mov edx, [PWM_RP_R]
		cmp edx, [PWM_WP_R]
		je short .RL_Lx_Empty

		mov ax, [PWM_FIFO_R + edx * 2]
		inc edx
		mov [PWM_Out_L], ax
		and edx, byte (PWM_BUF_SIZE - 1)
		mov [PWM_RP_R], edx
		pop edx
		pop eax
		ret

	ALIGN4

	.RL_Lx_Empty
		pop edx
		pop eax
		ret


	ALIGN32

	.RR_Lx
		mov edx, [PWM_RP_R]
		cmp edx, [PWM_WP_R]
		je short .RR_Lx_Empty

		mov ax, [PWM_FIFO_R + edx * 2]
		inc edx
		mov [PWM_Out_R], ax
		and edx, byte (PWM_BUF_SIZE - 1)
		mov [PWM_RP_R], edx
		pop edx
		pop eax
		ret

	ALIGN4

	.RR_Lx_Empty
		pop edx
		pop eax
		ret


	ALIGN32

	.RR_LL
		mov edx, [PWM_RP_L]
		cmp edx, [PWM_WP_L]
		je short .RR_LL_Left_Empty

		mov ax, [PWM_FIFO_L + edx * 2]
		inc edx
		mov [PWM_Out_L], ax
		and edx, byte (PWM_BUF_SIZE - 1)
		mov [PWM_RP_L], edx

	.RR_LL_Left_Empty
		mov edx, [PWM_RP_R]
		cmp edx, [PWM_WP_R]
		je short .RR_LL_Right_Empty

		mov ax, [PWM_FIFO_R + edx * 2]
		inc edx
		mov [PWM_Out_R], ax
		and edx, byte (PWM_BUF_SIZE - 1)
		mov [PWM_RP_R], edx
		pop edx
		pop eax
		ret

	ALIGN4
	
	.RR_LL_Right_Empty
		pop edx
		pop eax
		ret


	ALIGN32

	.RL_LR
		mov edx, [PWM_RP_L]
		cmp edx, [PWM_WP_L]
		je short .RL_LR_Left_Empty

		mov ax, [PWM_FIFO_L + edx * 2]
		inc edx
		mov [PWM_Out_R], ax
		and edx, byte (PWM_BUF_SIZE - 1)
		mov [PWM_RP_L], edx

	.RL_LR_Left_Empty
		mov edx, [PWM_RP_R]
		cmp edx, [PWM_WP_R]
		je short .RL_LR_Right_Empty

		mov ax, [PWM_FIFO_R + edx * 2]
		inc edx
		mov [PWM_Out_L], ax
		and edx, byte (PWM_BUF_SIZE - 1)
		mov [PWM_RP_R], edx
		pop edx
		pop eax
		ret

	ALIGN4
	
	.RL_LR_Right_Empty
		pop edx
		pop eax
		ret


	ALIGN32

	.Rx_Lx
		pop edx
		pop eax
		ret


	ALIGN32

	; void PWM_Update(int **buf, int lenght)
	DECLF PWM_Update, 8

		push ebx
		push esi

		test byte [PWM_Enable], 0xFF
		mov eax, [PWM_Out_L]
		jz short .End

		mov ebx, ecx
		shl eax, 5
		mov ecx, edx
		mov edx, [PWM_Out_R]
		and eax, 0xFFFF
		shl edx, 5
		mov esi, [ebx + 4]
		and edx, 0xFFFF
		sub eax, 0x4000
		sub edx, 0x4000
		test ecx, ecx
		mov ebx, [ebx]
		jnz short .Loop

		pop esi
		pop ebx
		ret

	ALIGN32

	.Loop
		add [ebx], eax
		add [esi], edx
		add ebx, byte 4
		add esi, byte 4
		dec ecx
		jnz short .Loop

	.End
		pop esi
		pop ebx
		ret