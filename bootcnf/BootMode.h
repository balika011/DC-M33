
#ifndef BOOT_MODE_H
#define BOOT_MODE_H

#include <vector>

#include "ModuleEntry.h"

class CBootMode
{
public:
	CBootMode();
	CBootMode(int Mode1, int Mode2);
	~CBootMode();

	int ParseBinary(char *buffer);

	void WriteBinary(FILE *fd);

	int GetMode(void)
	{
		return mMode1;
	}
	
	void SetModuleCount(int ModCount)
	{
		mModCount = ModCount;
	}

	void Print(void);

private:
	int mMode1;
	int mMode2;
	int mModCount;

	struct ModeBinHeader
	{
		int ModCount;
		int Mode1;
		int Mode2;
		int zero[5];
	};
};

#endif
