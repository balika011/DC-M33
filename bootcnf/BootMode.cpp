
#include <stdio.h>
#include <string.h>

#include "BootMode.h"

CBootMode::CBootMode()
{
	mMode1 = 0;
	mMode2 = 0;
	mModCount = 0;
}

CBootMode::CBootMode(int Mode1, int Mode2)
{
	mMode1 = Mode1;
	mMode2 = Mode2;
	mModCount = 0;
}

CBootMode::~CBootMode()
{
}

int CBootMode::ParseBinary(char *buffer)
{
	struct ModeBinHeader *modeHeader = (ModeBinHeader *)buffer;

	mModCount = modeHeader->ModCount;
	mMode1 = modeHeader->Mode1;
	mMode2 = modeHeader->Mode2;

	return 0;
}

void CBootMode::WriteBinary(FILE *fd)
{
	struct ModeBinHeader hdr;
	memset(&hdr, 0, sizeof(hdr));

	hdr.Mode1 = mMode1;
	hdr.Mode2 = mMode2;
	hdr.ModCount  = mModCount;

	fwrite(&hdr, sizeof(hdr), 1, fd);
}

void CBootMode::Print(void)
{
	printf("CBootMode\tmMode1=0x%x\n", mMode1);
	printf("CBootMode\tmMode2=0x%x\n", mMode2);
	printf("CBootMode\tmModCount=%d\n", mModCount);
}
