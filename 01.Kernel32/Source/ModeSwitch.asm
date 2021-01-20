[BITS32]	;���� �ڵ� 32��Ʈ �ڵ�� ����

;C���� ȣ�Ⱑ���ϵ��� �̸� ����(Export)
global kReadCPUID, kSwitchAndExecute64BitKernel

SECTION .text	;text ����(���׸�Ʈ) ����

; CPUID�� ��ȯ
; PARAM: DWORD dwEAX, DWORD* pdwEAX,* pdwEBX,* pdeECX,* pdwEDX
kReadCPUID:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx
	push esi

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;EAX �������� ������ CPUID ����
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, dword[ebp + 8]	;�Ķ���� 1(dwEAX)�� �������Ϳ� ����
	cpuid					;CPUID ��ɾ� ����
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;��ȯ�� ���� �Ķ���Ϳ� ����
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; *pdwEAX
	mov esi, dword[ebp + 12]
	mov dword[esi], eax

	; *pdwEBX
	mov esi dword[ebp + 16]
	mov dword[esi], ebx

	; *pdwEBX
	mov esi dword[ebp + 20]
	mov dword[esi], ecx

	; *pdwEBX
	mov esi dword[ebp + 24]
	mov dword[esi], edx

	pop esi		;�Լ����� ��� ���� �������͵� �� ����
	pop edx
	pop ecx
	pop ebx
	pop eax
	pop ebp
	ret			;�Լ� ȣ���� ���� �ڵ�� ����

;IA-32e���� ��ȯ�ϰ� 64��Ʈ Ŀ�� ����
;PARAM: ����
kSwitchAndExecute64bitKernel
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;CR4 ��Ʈ�� ���������� PAE ��Ʈ 1�� ����
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, cr4
	or eax, 0x20
	mov cr4, eax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;CR3 ��Ʈ�� �������Ϳ� PML4 ���̺��� �ּҿ� ĳ�� Ȱ��ȭ
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, 0x100000
	mov cr3, eax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;IA32_EFER.LME�� 1�� �����Ͽ� IA-32e��带 Ȱ��ȭ
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ecx, 0xC0000080	;IA32_EFER MSR �������� �ּ�.
	rdmsr				;MSR �������� �б�. eax �������ͷ� ��ȯ��.
	or eax, 0X0100		;LME��Ʈ(��Ʈ8) Ȱ��ȭ
	wrmsr

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;ĳ��, ����¡ ��� Ȱ��ȭ
	;CRO�� NW(��Ʈ 29) = 0, CD(��Ʈ 30) = 0, PG(��Ʈ 31) = 1
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, cr0
	or eax, 0xE0000000	;NW, CD, PG�� ��� 1�� ����
	xor eax, 0x60000000	;NW = 0, CD = 0, PG = 1
	mov cr0, eax		;�� ����

	jmp 0x08:0x200000	;CS ���׸�Ʈ �����Ϳ� IA-32e���� �ڵ� ���׸�Ʈ ��ũ���� �Ҵ�.
						;0x200000 �ּҷ� �̵�

	;����� ������� ����
	jmp $
