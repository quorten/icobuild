/* icobuild.c -- builds a Windows icon based off of a set of image files.
    * Does not currently support animated cursors
    * Cursors will only allow 1 image
   Compiling: Struct alignment must be set to 1 byte (this should be
   automatic from the header files). Otherwise, the bitmap headers
   will not load properly.  Integers must also be 32 bits by
   default.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "icoinfo.h"

int main(int argc, char* argv[])
{
	ICOINFOHEADER icoHead;
	ICODIRECTORY* icoDir;
	FILE* outFP;

	int numImages;
	unsigned currentOffset; /* Current end-of-file position in output */
	/* Temporary variables */
	int i;

	if (argc == 1 || argc == 2 &&
		(strcmp(argv[1] , "/?") == 0 ||
		strcmp(argv[1], "--help") == 0 ||
		strcmp(argv[1], "-h") == 0))
	{
		puts(
"Ussage: icobuild NUMIMAGES for icons\n\
        -OR-\n\
        icobuild HOTX HOTY for cursors\n\
Each XOR image should be named xor0.bmp, xor1.bmp, ... and each AND image\n\
should be named and0.bmp, and1.bmp, ...\n\
Output is written to \"output.ico\" or \"output.cur\".\n\
If you use a PNG image, it is assumed to be 256x256.\n\
Animated cursors are not supported.");
		return 0;
	}
	if (argc != 2 && argc != 3)
		return 0;

	if (argc == 3)
	{
		outFP = fopen("output.cur", "wb");
		if (outFP == NULL)
		{
			fprintf(stderr, "Error: Could not open \"output.cur\"\n");
			return 1;
		}
		puts("Warning: Animated cursors are not supported.");
		numImages = 1;
		icoHead.type = 2;
	}
	else
	{
		outFP = fopen("output.ico", "wb");
		if (outFP == NULL)
		{
			fprintf(stderr, "Error: Could not open \"output.ico\"\n");
			return 1;
		}
		numImages = atoi(argv[1]);
		icoHead.type = 1;
	}

	icoHead.reserved = 0;
	icoHead.count = numImages;
	icoDir = (ICODIRECTORY*)malloc(sizeof(ICODIRECTORY) * numImages);
	currentOffset = sizeof(ICOINFOHEADER) + sizeof(ICODIRECTORY) * numImages;

	fwrite(&icoHead, sizeof(ICOINFOHEADER), 1, outFP);

	for (i = 0; i < numImages; i++)
	{
		char filename[100];
		FILE* fp;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER* bih; /* This is polymorphic */
		RGBQUAD colorMasks[3];
		RGBQUAD colorTable[256];
		BYTE* data;

		unsigned clrTableMax;
		unsigned xorSize;
		unsigned andSize;
		/* lastPos is used as a file position to jump back to the
		   array of icon directories.  */
		unsigned lastPos;

		sprintf(filename, "xor%i.bmp", i);
		fp = fopen(filename, "rb");
		if (fp == NULL)
		{
			sprintf(filename, "xor%i.png", i);
			fp = fopen(filename, "rb");
			if (fp == NULL)
			{
				fprintf(stderr, "Error: Could not open \"%s\"\n", filename);
				free(icoDir);
				fclose(outFP);
				return 2;
			}
			else
			{
				/* Do custom handling for 256x256 PNG images.  */
				unsigned pngSize;
				fseek(fp, 0, SEEK_END);
				pngSize = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				data = (BYTE*)malloc(pngSize);
				fread(data, pngSize, 1, fp);
				fclose(fp);

				icoDir[i].width = 0; /* 256 */
				icoDir[i].height = 0; /* 256 */
				icoDir[i].colorCount = 0;
				icoDir[i].reserved = 0;
				icoDir[i].colorPlanes = 1;
				icoDir[i].bpp = 32;
				icoDir[i].imgSize = pngSize;
				icoDir[i].imgOffset = currentOffset;

				fwrite(&(icoDir[i]), sizeof(ICODIRECTORY), 1, outFP);
				lastPos = ftell(outFP);
				fseek(outFP, currentOffset, SEEK_SET);
				fwrite(data, pngSize, 1, outFP);
				free(data);
				currentOffset = ftell(outFP);
				fseek(outFP, lastPos, SEEK_SET);
				continue;
			}
		}

		fread(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
		/* Get the header size.  */
		{
			unsigned biSize;
			fread(&biSize, sizeof(unsigned), 1, fp);
			fseek(fp, -sizeof(unsigned), SEEK_CUR);
			bih = (BITMAPINFOHEADER*)malloc(biSize);
			bih->biSize = biSize;
		}
		fread(bih, bih->biSize, 1, fp);

		/* Read any color tables or bit fields.  */
		clrTableMax = 0;
		switch (bih->biBitCount)
		{
			case 1: clrTableMax = 2; break;
			case 4: clrTableMax = 16; break;
			case 8: clrTableMax = 256; break;
		}
		if (clrTableMax != 0)
		{
			if (bih->biClrUsed == 0)
				fread(colorTable, sizeof(RGBQUAD), clrTableMax, fp);
			else
				fread(colorTable, sizeof(RGBQUAD), bih->biClrUsed, fp);
		}
		if (bih->biSize == sizeof(BITMAPINFOHEADER) &&
			bih->biCompression == BI_BITFIELDS)
			fread(colorMasks, sizeof(RGBQUAD), 3, fp);
		if (bih->biBitCount > 8 && bih->biClrUsed != 0)
			fread(colorTable, sizeof(RGBQUAD), bih->biClrUsed, fp);

		/* Calculate the XOR and AND mask sizes.  */
		xorSize = bih->biWidth * bih->biBitCount;
		if (xorSize % 32 != 0) /* Keep scanlines DWORD aligned */
			xorSize = xorSize - (xorSize % 32) + 32;
		xorSize /= 8;
		xorSize *= bih->biHeight;
		andSize = bih->biWidth;
		if (andSize % 32 != 0)
			andSize = andSize - (andSize % 32) + 32;
		andSize /= 8;
		andSize *= bih->biHeight;

		/* Read the XOR mask.  */
		data = (BYTE*)malloc(xorSize);
		fseek(fp, bfh.bfOffBits, SEEK_SET);
		fread(data, xorSize, 1, fp);
		fclose(fp);

		/* Fill in the ICODIRECTORY information.  */
		icoDir[i].width = (BYTE)bih->biWidth;
		icoDir[i].height = (BYTE)bih->biHeight;
		if (clrTableMax < 256 || (bih->biClrUsed != 0 && bih->biClrUsed < 256))
		{
			if (bih->biClrUsed != 0)
				icoDir[i].colorCount = (BYTE)bih->biClrUsed;
			else
				icoDir[i].colorCount = clrTableMax;
		}
		else
			icoDir[i].colorCount = 0;
		icoDir[i].reserved = 0;
		if (argc == 3)
		{
			/* Set the hotspot X and Y coordinates.  */
			icoDir[i].colorPlanes = atoi(argv[1]);
			icoDir[i].bpp = atoi(argv[2]);
		}
		else
		{
			icoDir[i].colorPlanes = 1;
			icoDir[i].bpp = bih->biBitCount;
		}
		icoDir[i].imgSize = bih->biSize + xorSize + andSize;
		if (clrTableMax != 0)
		{
			if (bih->biClrUsed != 0)
				icoDir[i].imgSize += sizeof(RGBQUAD) * bih->biClrUsed;
			else
				icoDir[i].imgSize += sizeof(RGBQUAD) * clrTableMax;
		}
		if (bih->biSize == sizeof(BITMAPINFOHEADER) &&
			bih->biCompression == BI_BITFIELDS)
			icoDir[i].imgSize += sizeof(RGBQUAD) * 3;
		if (bih->biBitCount > 8 && bih->biClrUsed != 0)
			icoDir[i].imgSize += sizeof(RGBQUAD) * bih->biClrUsed;
		icoDir[i].imgOffset = currentOffset;
		bih->biHeight *= 2; /* Include AND mask height */
		/* Zero everything unimportant? (not in this code) */

		/* Write the ICODIRECTORY information.  */
		fwrite(&(icoDir[i]), sizeof(ICODIRECTORY), 1, outFP);

		/* Write the XOR mask.  */
		lastPos = ftell(outFP);
		fseek(outFP, currentOffset, SEEK_SET);
		fwrite(bih, bih->biSize, 1, outFP);
		if (clrTableMax != 0)
		{
			if (bih->biClrUsed != 0)
				fwrite(colorTable, sizeof(RGBQUAD), bih->biClrUsed, outFP);
			else
				fwrite(colorTable, sizeof(RGBQUAD), clrTableMax, outFP);
		}
		if (bih->biSize == sizeof(BITMAPINFOHEADER) &&
			bih->biCompression == BI_BITFIELDS)
			fwrite(colorMasks, sizeof(RGBQUAD), 3, outFP);
		if (bih->biBitCount > 8 && bih->biClrUsed != 0)
			fwrite(colorTable, sizeof(RGBQUAD), bih->biClrUsed, outFP);
		fwrite(data, xorSize, 1, outFP);
		free(bih);
		free(data);

		/* Read the AND mask.  */
		sprintf(filename, "and%i.bmp", i);
		fp = fopen(filename, "rb");
		if (fp == NULL)
		{
			fprintf(stderr, "Error: Could not open \"%s\"\n", filename);
			free(icoDir);
			fclose(outFP);
			return 3;
		}
		fread(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
		fseek(fp, bfh.bfOffBits, SEEK_SET);
		data = (BYTE*)malloc(andSize);
		fread(data, andSize, 1, fp);
		fclose(fp);

		/* Write the AND mask.  */
		fwrite(data, andSize, 1, outFP);
		free(data);
		currentOffset = ftell(outFP);
		fseek(outFP, lastPos, SEEK_SET);
	}
	free(icoDir);
	fclose(outFP);

	return 0;
}
