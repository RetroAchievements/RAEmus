%include "nasmhead.inc"


section .data align=64



section .bss align=64

	extern MD_Screen
	
	DECL _32X_Palette_16B
	resw 0x10000

	DECL _32X_VDP_Ram
	resb 0x100 * 1024

	DECL _32X_VDP_CRam
	resw 0x100

	DECL _32X_VDP_CRam_Ajusted
	resw 0x100

	ALIGNB 32
	
	DECL _32X_VDP
	.Mode		resd 1
	.State		resd 1
	.AF_Data	resd 1
	.AF_St		resd 1
	.AF_Len		resd 1
	.AF_Line	resd 1



section .text align=64

	; void _32X_VDP_Reset()
	DECLF _32X_VDP_Reset

		push ecx
		push edi
		
		xor eax, eax
		mov ecx, (256 * 1024 / 4)
		mov [_32X_VDP.Mode], eax
		mov edi, _32X_VDP_Ram
		mov [_32X_VDP.State], eax
		mov [_32X_VDP.AF_Data], eax
		rep stosd
		mov [_32X_VDP.AF_St], eax
		mov [_32X_VDP.AF_Len], eax
		mov [_32X_VDP.AF_Line], eax

		pop edi
		pop ecx
		ret


	ALIGN32

	; void _32X_VDP_Draw(int FB_Num)
	DECLF _32X_VDP_Draw

		mov eax, [esp + 4]
		pushad
		and eax, byte 1
		lea edi, [MD_Screen + 8 * 2]
		shl eax, 17
		xor ebp, ebp
		lea esi, [eax + _32X_VDP_Ram]
		xor ebx, ebx

	.Loop_Y
		mov eax, [_32X_VDP.Mode]
		mov bx, [esi + ebp * 2]
		and eax, byte 3
		mov ecx, 160
		jmp [.Table_32X_Draw + eax * 4]

	ALIGN4

	.Table_32X_Draw

		dd .32X_Draw_M00, .32X_Draw_M01, .32X_Draw_M10, .32X_Draw_M11

	ALIGN32

	.32X_Draw_M10
	.32X_Draw_M10_P
			movzx eax, word [esi + ebx * 2 + 0]
			movzx edx, word [esi + ebx * 2 + 2]
			mov ax, [_32X_Palette_16B + eax * 2]
			mov dx, [_32X_Palette_16B + edx * 2]
			mov [edi + 0], ax
			mov [edi + 2], dx
			add bx, byte 2
			add edi, byte 4
			dec ecx
			jnz short .32X_Draw_M10
			jmp .End_Loop_Y

	.32X_Draw_M00_P
	.32X_Draw_M00
		popad
		ret

	ALIGN32

	.32X_Draw_M01
	.32X_Draw_M01_P
			movzx eax, byte [esi + ebx * 2 + 0]
			movzx edx, byte [esi + ebx * 2 + 1]
			mov ax, [_32X_VDP_CRam_Ajusted + eax * 2]
			mov dx, [_32X_VDP_CRam_Ajusted + edx * 2]
			mov [edi + 2], ax
			mov [edi + 0], dx
			inc bx
			add edi, byte 4
			dec ecx
			jnz short .32X_Draw_M01
			jmp .End_Loop_Y

	ALIGN32

	.32X_Draw_M11
	.32X_Draw_M11_P
			mov edx, 320
			jmp short .32X_Draw_M11_Loop

	ALIGN4
	
		.32X_Draw_M11_Loop
			movzx eax, byte [esi + ebx * 2 + 0]
			movzx ecx, byte [esi + ebx * 2 + 1]
			mov ax, [_32X_VDP_CRam_Ajusted + eax * 2]
			inc ecx
			inc bx
			sub edx, ecx
			jbe short .32X_Draw_M11_End
			rep stosw
			jmp short .32X_Draw_M11_Loop

	ALIGN4

	.32X_Draw_M11_End
		add ecx, edx
		rep stosw

	.End_Loop_Y
		inc ebp
		add edi, byte 8 * 2 * 2
		cmp ebp, 220
		jb near .Loop_Y

		lea edi, [MD_Screen + 8 * 2 + 336 * 2 * 220]
		xor eax, eax
		mov ecx, 128

	.Palette_Loop
			mov dx, [_32X_VDP_CRam_Ajusted + eax * 2]
			mov [edi + 0], dx
			mov [edi + 2], dx
			mov [edi + 336 * 2 + 0], dx
			mov [edi + 336 * 2 + 2], dx
			mov dx, [_32X_VDP_CRam_Ajusted + eax * 2 + 128 * 2]
			mov [edi + 336 * 4 + 0], dx
			mov [edi + 336 * 4 + 2], dx
			mov [edi + 336 * 6 + 0], dx
			mov [edi + 336 * 6 + 2], dx
			add edi, byte 4
			inc eax
			dec ecx
			jnz short .Palette_Loop 

		popad
		ret