#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);
void kCopyKernel64ImageTo2Mbyte(void);

//Main
void Main(void)
{
	DWORD i;
	DWORD dwEAX, dwEBX, dwECX, dwEDX;
	char vcVendorString[13] = {0};
	kPrintString(0,3,"Protected Mode C Language Kernel Start......[Pass]");
	//최소 메모리 크기 만족하는지 검사
	kPrintString(0,4,"Minimum Memory Size Check...................[    ]");
	if(kIsMemoryEnough() == FALSE)
	{
		kPrintString(45,4,"Fail");
		kPrintString(0,5,"Not Enough Memory. MINT64 OS Requires Over 64Mbyte Memory");
		while(1);
	}
	else
	{
		kPrintString(45,4,"Pass");
	}
	//IA-32e모드의 커널영역 초기화
	kPrintString(0,5,"IA-32e Kernel Area Initialize...............[    ]");
	if(kInitializeKernel64Area() == FALSE)
	{
		kPrintString(45,5,"Fail");
		kPrintString(0,6,"Kernel Area Initialization Fail.");
		while(1);
	}
	kPrintString(45,5,"Pass");

	//IA-32e 모드 커널을 위한 페이지 테이블 생성
	kPrintString(0,6,"IA-32e Page Tables Initialize...............[    ]");
	kInitializePageTables();
	kPrintString(45, 6, "Pass");

	//프로세서 제조사 정보 읽기
	kReadCPUID(0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX);

	*(DWORD*)vcVendorString = dwEBX;
	*((DWORD*)vcVendorString + 1) = dwEDX;
	*((DWORD*)vcVendorString + 2) = dwECX;
	kPrintString(0,7,"Processor Vendor String.....................[            ]");
	kPrintString(45, 7, vcVendorString);

	//64비트 지원 유무 확인
	kReadCPUID(0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX);
	kPrintString(0, 8, "64bit Mode Support Check....................[    ]");
	if (dwEDX & (1 << 29)) // 비트 29가 1인지 확인
	{
		kPrintString(45, 8, "Pass");
	}
	else
	{
		kPrintString(45, 8, "Fail");
		kPrintString(0, 9, "This Processor Does Not Support 64Bit Mode");
		while (1);	
	}

	//IA-32e 모드 커널을 0x200000(2MB) 주소로 이동
	kPrintString(0, 9, "Copy IA-32e Kernel To 2M Address............[    ]");
	kCopyKernel64ImageTo2Mbyte();
	kPrintString(45, 9, "Pass");

	//IA-32e 모드로 전환
	kPrintString(0, 9, "Switch To IA-32e Mode");
	kSwitchAndExecute64bitKernel();

	while(1);
}

//문자 출력 함수
void kPrintString(int iX, int iY, const char* pcString)
{
	CHARACTER* pstScreen = (CHARACTER*)0xB8000;
	int i;

	pstScreen += (iY*80) + iX;

	for(i = 0; pcString[i] != 0; i++)
		pstScreen[i].bCharactor = pcString[i];
}

//IA-32e모드용 커널영역 0으로 초기화
BOOL kInitializeKernel64Area(void)
{
	DWORD* pdwCurrentAddress;

	//초기화 시작할 주소 설정
	pdwCurrentAddress = (DWORD*) 0x100000;

	//마지막 주소까지 루프 돌면서 4바이트씩 0으로 채움.
	while((DWORD)pdwCurrentAddress < 0x600000)
	{
		*pdwCurrentAddress = 0x00;

		//0으로 저장 후 다시 읽었을 때 0이 나오지 않으면 해당 주소를
		//사용하는데 문제가 생긴것이므로 중단하고 종료.
		if(*pdwCurrentAddress != 0)
		{
			return FALSE;
		}

		//다음 주소로 이동. 4바이트+
		pdwCurrentAddress++;
	}
	return TRUE;
}

//MINTOS를 실행하기에 충분한 메모리를 가지고 있는지 채크
BOOL kIsMemoryEnough(void)
{
	DWORD* pdwCurrentAddress;

	//0x100000(1MB)부터 검사 시작
	pdwCurrentAddress = (DWORD*) 0x100000;

	//0x4000000(64MB)까지 루프 돌며 확인
	while((DWORD) pdwCurrentAddress < 0x4000000)
	{
		*pdwCurrentAddress = 0x12345678;

		//0x12345678로 저장 후에 이게 안나오면
		//해당 주소 쓰는데 문제 있는것이므로 중단
		
		if(*pdwCurrentAddress != 0x12345678)
		{
			return FALSE;
		}

		//1MB씩 이동하며 확인
		pdwCurrentAddress += (0x100000/4);
	}
	return TRUE;
}

//IA-32e 모드 커널을 0x200000(2M)주소롤 복사
void kCopyKernel64ImageTo2Mbyte(void)
{
	WORD wKernel32SectorCount, wTotalKernelSectorCount;
	DWORD* pdwSourceAddress, * pdwDestinationAddress;
	int i;

	//0x7C05에 총 커널 섹터 수, 0x7C07에 보호모드 커널 섹터수 있음.
	wTotalKernelSectorCount = *((WORD*)0X7c05);
	wKernel32SectorCount = *((WORD*)0X7C07);

	pdwSourceAddress = (DWORD*)(0x10000 + (wKernel32SectorCount * 512));
	pdwDestinationAddress = (DWORD*)0x200000;
	
	//IA-32e 모드 커널 섹터 크기만큼 복사
	for (i = 0; i < 512 * (wTotalKernelSectorCount - wKernel32SectorCount) / 4; i++)
	{
		*pdwDestinationAddress = *pdwSourceAddress;
		pdwDestinationAddress++;
		pdwSourceAddress++;
	}
}

