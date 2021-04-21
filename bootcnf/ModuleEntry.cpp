
#include <stdio.h>
#include <string.h>

#include "ModuleEntry.h"

int GetBigLong(unsigned char *data_p)
{
	int val = 0;
	val = *(data_p)<<24;
	val = val + (*(data_p+1)<<16);
	val = val + (*(data_p+2)<<8);
	val = val + *(data_p+3);
	data_p += 4;
	return val;
}

CModuleEntry::CModuleEntry(void)
{
	mFlags		= 0;
	mNameOffset	= 0;

	memset(mKey, 0, 0x10);
}

CModuleEntry::~CModuleEntry()
{

}

int CModuleEntry::ParseBinary(char *buffer)
{
	struct ModEntryHeader *modHdr = (ModEntryHeader *)buffer;

	mFlags		= modHdr->Flags;
	mNameOffset	= modHdr->ModNameOffset;

	memcpy(mKey, modHdr->key, 0x10);

	return 0;
}

int CModuleEntry::ParseText(char *file, char *key, char *flags, char *modNames, int &namePos)
{
	char *str;

	if(file[0] == '$')
	{
		mFlags |= 0x80000000;
		file++;
	}

	if(file[0] == '%')
	{
		file++;
		if(file[0] != '%')
		{
			mFlags |= 0x20000;
		}
		else
		{
			mFlags |= 0x40000;
			file++;
		}
	}
	else
	{
		mFlags |= 0x10000;	
	}

	str = strstr(modNames, file);

	if(str == 0)
	{
		mNameOffset = namePos;

		strcpy(modNames+namePos, file);
		namePos += strlen(file);
		modNames[namePos]='*';
		namePos ++;
	}
	else
	{
		mNameOffset = str - modNames;
	}

	int tmp[4];

	sscanf(key, "%08X%08X%08X%08X\n", &tmp[0], 
									  &tmp[1], 
									  &tmp[2], 
									  &tmp[3]);

	mKey[0] = GetBigLong((unsigned char *)&tmp[0]);
	mKey[1] = GetBigLong((unsigned char *)&tmp[1]);
	mKey[2] = GetBigLong((unsigned char *)&tmp[2]);
	mKey[3] = GetBigLong((unsigned char *)&tmp[3]);
	
	for (;*flags;++flags)
	{
		if(*flags == 'V')
			mFlags |= 1;
		if(*flags == 'G')
			mFlags |= 2;
		if(*flags == 'U')
			mFlags |= 4;
		if(*flags == 'P')
			mFlags |= 8;
		if(*flags == 'L')
			mFlags |= 0x10;
		if(*flags == 'A')
			mFlags |= 0x20;
		if(*flags == 'D')
			mFlags |= 0x40;
		if(*flags == 'M')
			mFlags |= 0x80;
	}



	return 0;
}

int CModuleEntry::WriteBinary(FILE *fd)
{
	struct ModEntryHeader hdr;
	memset(&hdr, 0, sizeof(hdr));

	hdr.Flags = mFlags;

	hdr.ModNameOffset = mNameOffset;

	memcpy(hdr.key, mKey, 0x10);

	fwrite(&hdr, sizeof(hdr), 1, fd);

	return 0;
}

int CModuleEntry::WriteText(FILE *fd, char *modNames)
{
	if(mFlags & 0x80000000)
	{
		fprintf(fd, "$");
	}
	if(mFlags & 0x20000)
	{
		fprintf(fd, "%%");
	}
	else if(mFlags & 0x40000)
	{
		fprintf(fd, "%%%%");
	}

	fprintf(fd, "%s ", modNames+mNameOffset);

	fprintf(fd, "%08X%08X%08X%08X ", GetBigLong((unsigned char *)&mKey[0]), 
									  GetBigLong((unsigned char *)&mKey[1]), 
									  GetBigLong((unsigned char *)&mKey[2]), 							  GetBigLong((unsigned char *)&mKey[3]));
	if(mFlags & 1)
	{
		fprintf(fd, "V");
	}
	if(mFlags & 2)
	{
		fprintf(fd, "G");
	}
	if(mFlags & 4)
	{
		fprintf(fd, "U");
	}
	if(mFlags & 8)
	{
		fprintf(fd, "P");
	}
	if(mFlags & 0x10)
	{
		fprintf(fd, "L");
	}
	if(mFlags & 0x20)
	{
		fprintf(fd, "A");
	}
	if(mFlags & 0x40)
	{
		fprintf(fd, "D");
	}
	if(mFlags & 0x80)
	{
		fprintf(fd, "M");
	}
	
	fprintf(fd, "\n");

	return 0;
}

void CModuleEntry::Print(char *modNames)
{
	if(mFlags & 0x20000)
	{
		printf("%%");
	}
	else if(mFlags & 0x40000)
	{
		printf("%%%%");
	}

	printf("%s\n", modNames+mNameOffset);
}
