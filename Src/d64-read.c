#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ugly kludge to remove warnings. This file should really be fixed instead... */
#define fread(ptr, size, nmemb, stream) do { if (fread(ptr,size,nmemb,stream) != nmemb) printf("fread failed");} while(0)

/********************************************************************************************/
static long GetFilePos(int Track, int Sector)
{
	long pos;

	pos = 0;

	if (Track >= 1 && Track <= 17)
	{
		if (Sector > 20)
		{
			Sector = 20;
		}
		pos += (Track - 1) * 21 * 256;
		pos += Sector * 256;
	}
	else if (Track >= 18 && Track <= 24)
	{
		if (Sector > 18)
		{
			Sector = 18;
		}
		pos += 17 * 21 * 256;
		pos += (Track - 18) * 19 * 256;
		pos += Sector * 256;
	}
	else if (Track >= 25 && Track <= 30)
	{
		if (Sector > 17)
		{
			Sector = 17;
		}
		pos += 17 * 21 * 256;
		pos +=  7 * 19 * 256;
		pos += (Track - 25) * 18 * 256;
		pos += Sector * 256;
	}
	else if (Track >= 31 && Track <= 35)
	{
		if (Sector > 16)
		{
			Sector = 16;
		}
		pos += 17 * 21 * 256;
		pos +=  7 * 19 * 256;
		pos +=  6 * 18 * 256;
		pos += (Track - 31) * 17 * 256;
		pos += Sector * 256;
	}
	else if (Track >= 36 && Track <= 40)
	{
		if (Sector > 16)
		{
			Sector = 16;
		}
		pos += 17 * 21 * 256;
		pos +=  7 * 19 * 256;
		pos +=  6 * 18 * 256;
		pos +=  5 * 17 * 256;
		pos += (Track - 36) * 17 * 256;
		pos += Sector * 256;
	}
	else
	{
		return 0;
	}
	return pos;
}
/**********************************************************************************************************/
const char **DirD64(const char *FileName) //mode = 0: quiet mode
{
	long pos, newpos, oldpos;
	unsigned char buffer[256];
	unsigned char ext[256];
	unsigned char name[256];
	unsigned char quotname[256];
	unsigned char blocks[256];
	unsigned char Type;
	unsigned char Track;
	unsigned char Sector;
	unsigned char track;
	unsigned char sector;
	unsigned char blow;
	unsigned char bhigh;
	int pblocks;
	unsigned int bfree;
	unsigned int f64count = 0;
	unsigned int Temp, i, j, k, totalblocks;
	const char **file_list;
	FILE *File;
	int cnt = 16;
	int cur = 0;

	File = fopen(FileName, "rb");
	if (!File)
	{
		return NULL;
	}

	file_list = (const char**)malloc(cnt * sizeof(char*));
	file_list[cur] = strdup((char*)"Empty Disc");
	file_list[cur + 1] = NULL;

	pos = GetFilePos(18, 0);
	fseek(File, pos, SEEK_SET);

	Track = 0;
	Sector = 0;
	fread(&Track, 1, 1, File);
	fread(&Sector, 1, 1, File);

	fread(buffer, 2, 1, File);

	bfree = 0;
	k = 0;
	for (j = 1; j <= 17; j++)
	{
		i = 0;
		fread(&i, 1, 1, File);
		bfree += i;
		fread(&k, 1, 1, File);
		fread(&k, 1, 1, File);
		fread(&k, 1, 1, File);

	}
	fread(&i, 1, 1, File);
	fread(&i, 1, 1, File);
	fread(&i, 1, 1, File);
	fread(&i, 1, 1, File);
	for (j = 19; j <= 24; j++)
	{
		i = 0;
		fread(&i, 1, 1, File);
		bfree += i;
		fread(&k, 1, 1, File);
		fread(&k, 1, 1, File);
		fread(&k, 1, 1, File);

	}
	for (j = 25; j <= 30; j++)
	{
		i = 0;
		fread(&i, 1, 1, File);
		bfree += i;
		fread(&k, 1, 1, File);
		fread(&k, 1, 1, File);
		fread(&k, 1, 1, File);

	}
	for (j = 31; j <= 35; j++)
	{
		i = 0;
		fread(&i, 1, 1, File);
		bfree += i;
		fread(&k, 1, 1, File);
		//		bfree += 0xFF - k;
		fread(&k, 1, 1, File);
		//		bfree += 0xFF - k;
		fread(&k, 1, 1, File);
		//		bfree += 0x01 - k;

	}
	/*
    1-17   21 sectors * 256 bytes
   18-24   19 sectors * 256 bytes
   25-30   18 sectors * 256 bytes
   31-35   17 sectors * 256 bytes
	 */

	fread(buffer, 16, 1, File);

	buffer[16] = 0;
	for (i = 0; i < 16; i++)
	{
		if (buffer[i] == 0xa0)
			buffer[i] = 0;
	}
	//	SetDlgItemText(hHandle, IDC_DISKTITLE, buffer);

	fread(buffer, 16, 1, File);
	fread(buffer, 16, 1, File);
	for (j = 36; j <= 40; j++)
	{
		i = 0;
		fread(&i, 1, 1, File);
		bfree += i;
		fread(&k, 1, 1, File);
		fread(&k, 1, 1, File);
		fread(&k, 1, 1, File);

	}
	pos = GetFilePos(18, 1);
	totalblocks = 0;
	do
	{
		if (fseek(File, pos, SEEK_SET))
		{
			break;
		}
		Track = 0;
		Sector = 0;
		track = 0;
		sector = 0;
		Type = 0;
		fread(&Track, 1, 1, File);
		fread(&Sector, 1, 1, File);
		pos = GetFilePos((int)Track, (int)Sector);
		for (i = 0; i < 8; i++)
		{
			fread(&Type, 1, 1, File);
			fread(&track, 1, 1, File);
			fread(&sector, 1, 1, File);
			fread(&name, 16, 1, File);
			name[16] = 0;

			for (j = 0; j < 16; j++)
			{
				if (name[j] == 0xa0)
				{
					name[j] = 0;
				}
				else if (name[j] > 0x7a)
				{
					//					name[j] = '_';
				}
			}

			fread(buffer, 9, 1, File);
			Temp = 0;
			fread(&blow, 1, 1, File);
			fread(&bhigh, 1, 1, File);
			Temp = 256 * bhigh+blow;
			fread(buffer, 2, 1, File);
			if (Type == 0x00)
				continue;
			switch (Type)
			{
			case 0x82:
				strcpy((char*)ext, "prg");
				break;
			case 0x81:
				strcpy((char*)ext, "seq");
				break;
			case 0x80:
				strcpy((char*)ext, "del");
				break;
			case 0x84:
				strcpy((char*)ext, "rel");
				break;
			case 0x83:
				strcpy((char*)ext, "user");
				break;
			case 0xc2:
				strcpy((char*)ext, "prg<");
				break;
			case 0xc1:
				strcpy((char*)ext, "seq<");
				break;
			case 0xc0:
				strcpy((char*)ext, "del<");
				break;
			case 0xc4:
				strcpy((char*)ext, "rel<");
				break;
			case 0xc3:
				strcpy((char*)ext, "user<");
				break;
			default:
				strcpy((char*)ext, "user<");
				break;
			}
			f64count ++;
			totalblocks += Temp;
			pblocks = (int)Temp;
			sprintf((char*)blocks, "%d", Temp);

			newpos = GetFilePos((int)track, (int)sector);
			if (newpos)
			{
				oldpos = ftell(File);

				if (!fseek(File, newpos, SEEK_SET))
				{
					fread(&Temp, 2, 1, File);
					fread(&Temp, 2, 1, File);
				}
				else
					break;

				if (fseek(File, oldpos, SEEK_SET))
					break;
			}

			sprintf((char*)quotname, "\"%s\"", name);
			sprintf((char*)buffer, "%-4d%-18s%c", pblocks, quotname, ext[0]);
			file_list[cur++] = strdup((char*)buffer);
			file_list[cur] = NULL;
			if (cur > cnt - 2)
			{
				cnt = cnt + 32;
				file_list = (const char**)realloc(file_list, cnt * sizeof(char*));
				if (!file_list)
					return NULL;
			}
		}
		if (f64count > 128)
			break;
	}
	while (Track);

	fclose(File);
	return file_list;

	//	sprintf(buffer, "%d blocks free.", 664 - totalblocks);
	//	SetDlgItemText(hHandle, IDC_FREEBLOCKS, buffer);

}
