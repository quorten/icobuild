/* icoextract.c -- extracts bitmap files from a Windows icon or cursor
   file.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "icoinfo.h"

enum bool_t {false, true};
typedef enum bool_t bool;

int main(int argc, char* argv[])
{
	ICOINFOHEADER icoHead;
	ICODIRECTORY icoDir;
	FILE* outFP; /* In this case, this is the input icon file.  */
	FILE* hotSpotsFP;
	bool isCursor = false;
	/* Temporary variables */
	unsigned i;

	if (argc == 1 || argc == 2 &&
		(strcmp(argv[1] , "/?") == 0 ||
		strcmp(argv[1], "--help") == 0 ||
		strcmp(argv[1], "-h") == 0))
	{
		puts(
"Ussage: icoextract FILE\n\
FILE is either an icon file or a non-animated cursor file.");
		return 0;
	}
	if (strlen(argv[1]) >= 3 &&
		*(argv[1] + strlen(argv[1]) - 3) == 'c')
	{
		isCursor = true;
		hotSpotsFP = fopen("hotspots.txt", "a");
		if (hotSpotsFP == NULL)
		{
			fprintf(stderr, "Error: Could not open \"hotspots.txt\"\n");
			return 2;
		}
	}

	outFP = fopen(argv[1], "rb");
	if (outFP == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[1]);
		return 1;
	}
	fread(&icoHead, sizeof(ICOINFOHEADER), 1, outFP);

	for (i = 0; i < icoHead.count; i++)
	{
		BYTE* imgData;
		unsigned lastPos;

		const char pngMagic[9] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A,
								  0x1A, 0x0A};
		char magicTest[8];

		BITMAPFILEHEADER bfh;
		unsigned width, height, bpp, clrUsed, compression, andSize;
		unsigned biHeadSize;
		char filename[100];
		FILE* fp;

		unsigned clrTableMax;
		BITMAPINFOHEADER bih;
		const RGBQUAD palette[2] = {{0, 0, 0, 0}, {0xFF, 0xFF, 0xFF, 0}};

		fread(&icoDir, sizeof(ICODIRECTORY), 1, outFP);

		/* Check for a PNG image.  */
		lastPos = ftell(outFP);
		fseek(outFP, icoDir.imgOffset, SEEK_SET);
		fread(magicTest, 8, 1, outFP);
		if (strncmp(magicTest, pngMagic, 8) == 0)
		{
			sprintf(filename, "xor%i.png", i);
			fp = fopen(filename, "wb");
			if (fp == NULL)
			{
				fprintf(stderr, "Error: Could not open \"%s\"\n", filename);
				free(imgData);
				if (isCursor == true)
					fclose(hotSpotsFP);
				fclose(outFP);
				return 3;
			}
			fseek(outFP, -8, SEEK_CUR);
			imgData = (BYTE*)malloc(icoDir.imgSize);
			fread(imgData, icoDir.imgSize, 1, outFP);
			fwrite(imgData, icoDir.imgSize, 1, fp);
			free(imgData);
			fclose(fp);
			continue;
		}
		else
			fseek(outFP, lastPos, SEEK_SET);

		/* Load the XOR and AND masks.  */
		imgData = (BYTE*)malloc(icoDir.imgSize);
		lastPos = ftell(outFP);
		fseek(outFP, icoDir.imgOffset, SEEK_SET);
		fread(imgData, icoDir.imgSize, 1, outFP);
		fseek(outFP, lastPos, SEEK_SET);

		biHeadSize = ((BITMAPINFOHEADER*)imgData)->biSize;
		width = ((BITMAPINFOHEADER*)imgData)->biWidth;
		height = (((BITMAPINFOHEADER*)imgData)->biHeight /= 2);
		bpp = ((BITMAPINFOHEADER*)imgData)->biBitCount;
		clrUsed = ((BITMAPINFOHEADER*)imgData)->biClrUsed;
		compression = ((BITMAPINFOHEADER*)imgData)->biCompression;

		/* Calculate the AND mask size.  */
		andSize = width;
		if (andSize % 32 != 0) /* Keep scanlines DWORD aligned */
			andSize = andSize - (andSize % 32) + 32;
		andSize /= 8;
		andSize *= height;

		/* Prepare the XOR mask bitmap header.  */
		bfh.bfType = 0x4D42; /* "BM" byteswapped */
		bfh.bfSize = sizeof(BITMAPFILEHEADER) + icoDir.imgSize - andSize;
		bfh.bfReserved1 = 0;
		bfh.bfReserved2 = 0;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + biHeadSize;

		/* Prepare color table or bit field header information.  */
		clrTableMax = 0;
		switch (bpp)
		{
			case 1: clrTableMax = 2; break;
			case 4: clrTableMax = 16; break;
			case 8: clrTableMax = 256; break;
		}
		if (clrTableMax != 0)
		{
			if (clrUsed == 0)
				bfh.bfOffBits += sizeof(RGBQUAD) * clrTableMax;
			else
				bfh.bfOffBits += sizeof(RGBQUAD) * clrUsed;
		}
		if (biHeadSize == sizeof(BITMAPINFOHEADER) &&
			compression == BI_BITFIELDS)
			bfh.bfOffBits += sizeof(RGBQUAD) * 3;
		if (bpp > 8 && clrUsed != 0)
			bfh.bfOffBits += sizeof(RGBQUAD) * clrUsed;

		/* Save the XOR mask.  */
		sprintf(filename, "xor%i.bmp", i);
		fp = fopen(filename, "wb");
		if (fp == NULL)
		{
			fprintf(stderr, "Error: Could not open \"%s\"\n", filename);
			free(imgData);
			if (isCursor == true)
				fclose(hotSpotsFP);
			fclose(outFP);
			return 3;
		}
		fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
		fwrite(imgData, bfh.bfSize - sizeof(BITMAPFILEHEADER), 1, fp);
		fclose(fp);

		/* Prepare the AND mask bitmap headers.  */
		bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
					 sizeof(RGBQUAD) * 2 + andSize;
		bfh.bfOffBits = bfh.bfSize - andSize;

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = width;
		bih.biHeight = height;
		bih.biPlanes = 1;
		bih.biBitCount = 1;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = andSize;
		bih.biXPelsPerMeter = ((BITMAPINFOHEADER*)imgData)->biXPelsPerMeter;
		bih.biYPelsPerMeter = ((BITMAPINFOHEADER*)imgData)->biYPelsPerMeter;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		/* Save the monochrome AND mask.  */
		sprintf(filename, "and%i.bmp", i);
		fp = fopen(filename, "wb");
		if (fp == NULL)
		{
			fprintf(stderr, "Error: Could not open \"%s\"\n", filename);
			free(imgData);
			if (isCursor == true)
				fclose(hotSpotsFP);
			fclose(outFP);
			return 4;
		}
		fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
		fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);
		fwrite(palette, sizeof(RGBQUAD) * 2, 1, fp);
		fwrite(imgData + icoDir.imgSize - andSize, andSize, 1, fp);
		fclose(fp);
		free(imgData);

		/* Save a text file with the hotspots (if a cursor).  */
		if (isCursor == true)
			fprintf(hotSpotsFP, "(%i, %i)\n", icoDir.colorPlanes, icoDir.bpp);
	}

	if (isCursor == true)
		fclose(hotSpotsFP);
	fclose(outFP);

	return 0;
}
