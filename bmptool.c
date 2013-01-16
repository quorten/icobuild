/* bmptool.c -- Converts 32-bit bitmap files to use BITMAPINFOHEADER as
   the bitmap info header and sets biCompression to BI_RGB.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "icoinfo.h"

int main(int argc, char* argv[])
{
	FILE* fp;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	BYTE* imgData;
	unsigned imgSize;

	if (argc == 1 || argc == 2 &&
		(strcmp(argv[1] , "/?") == 0 ||
		strcmp(argv[1], "--help") == 0 ||
		strcmp(argv[1], "-h") == 0))
	{
		puts(
"Ussage: bmptool FILE\n\
Converts 32-bit bitmap files to use BITMAPINFOHEADER as the bitmap\n\
info header and set biCompression to BI_RGB.");
		return 0;
	}

	fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[1]);
		return 1;
	}

	fread(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&bih, sizeof(BITMAPINFOHEADER), 1, fp);

	if (bih.biBitCount != 32)
	{
		fprintf(stderr, "Error: This bitmap is not eligible for this tool.\n");
		return 2;
	}

	/* 32-bit bitmaps always have their scanlines DWORD aligned.  */
	imgSize = bih.biWidth * bih.biHeight * (bih.biBitCount / 8);

	imgData = (BYTE*)malloc(imgSize);
	fseek(fp, bfh.bfOffBits, SEEK_SET);
	fread(imgData, imgSize, 1, fp);
	fclose(fp);

	/* Change the necessary portions of the headers.  */
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
		imgSize;
	bfh.bfOffBits = bfh.bfSize - imgSize;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biCompression = BI_RGB;
	bih.biSizeImage = 0;

	fp = fopen(argv[1], "wb");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[1]);
		free(imgData);
		return 1;
	}

	fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);
	fwrite(imgData, imgSize, 1, fp);
	fclose(fp);

	free(imgData);

	return 0;
}
