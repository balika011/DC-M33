
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BootConfig.h"

///////////////////////////////////////////////////////////////////////////////////////
//
// Constructors/Destructors
// 
///////////////////////////////////////////////////////////////////////////////////////
CBootConfig::CBootConfig(void)
{
	mModeCount		= 0;
	mModuleCount	= 0;
	mModNameSize	= 0;
	mpModNames		= 0;
}

CBootConfig::~CBootConfig()
{
	if(mpModNames)
	{
		free(mpModNames);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Parsers - Binary and text
// 
///////////////////////////////////////////////////////////////////////////////////////
int CBootConfig::ParseBinary(const char *file)
{
	int i;
	char *modePtr;
	char *modulePtr;

	FILE *fd = fopen(file, "rb");

	// A temp buffer to allow for 50k binary
	char buffer[50*1024];

	if(!fd)
	{
		printf("Unable to open file\n");
		return -1;
	}

	int fileSize = fread(buffer, 1, 50*1024, fd);

	if(fileSize > 0)
	{
		struct BootBinHeader *bootHdr = (BootBinHeader *)buffer;

		// Deal with all the module modes
		mModeCount = bootHdr->ModeCount;
		
		for(i=0; i<mModeCount; i++)
		{
			mBootMode.push_back(CBootMode());

			modePtr = buffer + bootHdr->ModeStart + (i*0x20);

			if(mBootMode[i].ParseBinary(modePtr) < 0)
			{
				return -1;
			}
		}

		// Now to sort out the module definitions
		mModuleCount = bootHdr->ModuleCount;

		for(i=0; i<mModuleCount; i++)
		{
			mModule.push_back(CModuleEntry());

			modulePtr = buffer + bootHdr->ModuleStart + (i*0x20);

			if(mModule[i].ParseBinary(modulePtr) < 0)
			{
				return -1;
			}
		}

		// Now finally the module string table
		char *moduleNames = buffer + bootHdr->ModNameStart;
		int nameSize = bootHdr->ModNameEnd - bootHdr->ModNameStart;

		mpModNames = (char*)malloc(nameSize);
		mModNameSize = nameSize;

		memcpy(mpModNames, moduleNames, nameSize);
	}

	return 0;
}

int CBootConfig::ParseText(const char *file)
{
	int i;
	char *linePtr;

	FILE *fd = fopen(file, "r");

	// A temp buffer to allow for 50k text file
	char buffer[50*1024];
	char textbuf[50*1024];
	int textPos = 1;

	textbuf[0] = '*';

	if(!fd)
	{
		printf("Unable to open file\n");
		return -1;
	}

	int fileSize = fread(buffer, 1, 50*1024, fd);

	linePtr = buffer;

	if(fileSize > 0)
	{
		do
		{
			char file[255];
			char key[255];
			char flags[255];
			int i = sscanf(linePtr, "%s %s %s", file, key, flags);

			if(i > 0)
			{
				CModuleEntry tmpEntry;
				tmpEntry.ParseText(file, key, flags, textbuf, textPos);

				mModule.push_back(tmpEntry);

				mModuleCount++;
			}
			linePtr++;
			linePtr = strchr(linePtr, '\n');
		} while (linePtr);

		// Now to copy the text across to its correct location
		mModNameSize = textPos;
		mpModNames = (char*)malloc(mModNameSize);

		memcpy(mpModNames, textbuf, mModNameSize);

		for(i=0; i<mModNameSize; i++)
		{
			if(mpModNames[i] == '*')
			{
				mpModNames[i] = 0;
			}
		}
		
		bool vsh = false;
		bool game = false;
		bool updater = false;
		bool pops = false;
		bool license = false;
		bool app = false;
		bool umd = false;
		bool mln = false;
		
		for (int i = 0; i < mModuleCount; i++)
		{
			int flags = mModule[i].GetFlags();
			if (flags & 1)
			{
				vsh = true;
			}
			if (flags & 2)
			{
				game = true;
			}
			if (flags & 4)
			{
				updater = true;
			}
			if (flags & 8)
			{
				pops = true;
			}
			if (flags & 0x10)
			{
				license = true;
			}
			if (flags & 0x20)
			{
				app = true;
			}
			if (flags & 0x40)
			{
				umd = true;
			}
			if (flags & 0x80)
			{
				mln = true;
			}
		}
		
		if (vsh)
		{
			mBootMode.push_back(CBootMode(1, 2));
			mModeCount++;
		}
		if (game)
		{
			mBootMode.push_back(CBootMode(2, 1));
			mModeCount++;
		}
		if (updater)
		{
			mBootMode.push_back(CBootMode(4, 3));
			mModeCount++;
		}
		if (pops)
		{
			mBootMode.push_back(CBootMode(8, 4));
			mModeCount++;
		}
		if (license)
		{
			mBootMode.push_back(CBootMode(0x10, 5));
			mModeCount++;
		}
		if (app)
		{
			mBootMode.push_back(CBootMode(0x20, 6));
			mModeCount++;
		}
		if (umd)
		{
			mBootMode.push_back(CBootMode(0x40, 7));
			mModeCount++;
		}
		if (mln)
		{
			mBootMode.push_back(CBootMode(0x80, 8));
			mModeCount++;
		}
		
					
		for (int i = 0; i < mModeCount; i++)
		{
			for (int j = mModuleCount; j; j--)
			{
				if (mModule[j].GetFlags() & mBootMode[i].GetMode())
				{
					mBootMode[i].SetModuleCount(j + 1);
					break;
				}
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Write - Binary and text
// 
///////////////////////////////////////////////////////////////////////////////////////
int CBootConfig::WriteBinary(const char *file)
{
	int i;

	FILE *fd = fopen(file, "w+b");

	if(fd <= 0)
	{
		printf("Unable to open file\n");
		return -1;
	}

	BootBinHeader hdr;
	memset(&hdr, 0, sizeof(hdr));

	// Write the header
	hdr.FileHeader	= 0x0F803001;
	hdr.devkit = 0x05000010;
	hdr.unknown0[0] = 0x6B8B4567;
	hdr.unknown0[1] = 0x327B23C6;
	hdr.unknown1[0] = 0x643C9869;
	hdr.unknown1[1] = 0x66334873;
	hdr.unknown2[0] = 0x74B0DC51;
	hdr.unknown2[1] = 0x19495CFF;
	hdr.unknown3[0] = 0x2AE8944A;
	hdr.unknown3[1] = 0x625558EC;

	int offset = sizeof(hdr);
	hdr.ModeStart	= offset;
	hdr.ModeCount	= mModeCount;

	offset += 0x20 * mModeCount;
	hdr.ModuleStart	= offset;
	hdr.ModuleCount = mModuleCount;

	offset += 0x20 * mModuleCount;
	hdr.ModNameStart = offset;

	offset += mModNameSize;
	hdr.ModNameEnd   = offset;

	fwrite(&hdr, sizeof(hdr), 1, fd);

	for(i=0; i<mModeCount; i++)
	{
		mBootMode[i].WriteBinary(fd);
	}

	for(i=0; i<mModuleCount; i++)
	{
		mModule[i].WriteBinary(fd);
	}

	fwrite(mpModNames, mModNameSize, 1, fd);

	return 0;
}

int CBootConfig::WriteText(const char *file)
{
	FILE *fd = fopen(file, "w+");
		
	for(int i = 0; i < mModuleCount; i++)
	{
		mModule[i].WriteText(fd, mpModNames);
	}

	fclose(fd);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Debug
// 
///////////////////////////////////////////////////////////////////////////////////////
void CBootConfig::Print(void)
{
	printf("CBootConfig\tModeCount = %d\n\n", mModeCount);

	int i,j,mode;
	for(i=0; i<mModeCount; i++)
	{
		printf("CBootConfig\tMode %d\n", i);
		mBootMode[i].Print();
		
		mode = mBootMode[i].GetMode();
		printf("\n");
	}

	for(j=0; j<mModuleCount; j++)
	{
		mModule[j].Print(mpModNames);
		printf("\n");
	}
		
}
