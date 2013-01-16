/* Contains structures required for reading and writing data for this
   program */

/* Simple data definitions */
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG; /* ? */

/* #pragma ms-struct(push) */
#pragma pack(push,1)

/* Icon structures */
typedef struct tagICOINFOHEDAER
{
	WORD	reserved;	/* Should be 0 */
	WORD	type;		/* 1 for icon, 2 for cursor */
	WORD	count;		/* image count */
} ICOINFOHEADER;

typedef struct tagICODIRECTORY
{
	BYTE	width;
	BYTE	height;
	BYTE	colorCount;		/* Number of colors if less than 256 */
	BYTE	reserved;
	WORD	colorPlanes;	/* should be 0 or 1 or X hotspot */
	WORD	bpp;			/* or Y hotspot */
	DWORD	imgSize;
	DWORD	imgOffset;
} ICODIRECTORY;

/* Bitmap structures, copied from Windows Platform SDK */
/* See the Windows Platform SDK for more details */

/* Bitmap files begin with: */
typedef struct tagBITMAPFILEHEADER
{
	WORD	bfType; /* should equal "BM" */
	DWORD	bfSize; /* total file size */
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits; /* offset to bitmap bits */
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

/* Next comes: */
typedef struct tagBITMAPINFOHEADER
{
	DWORD	biSize;
	LONG	biWidth;
	LONG	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	LONG	biXPelsPerMeter; /* Pixels per Meter */
	LONG	biYPelsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

/* Constants for the biCompression field */
#define BI_RGB        0
#define BI_RLE8       1
#define BI_RLE4       2
#define BI_BITFIELDS  3
#define BI_JPEG       4
#define BI_PNG        5

/* Then comes the color palette or bitmasks (if there is/are one/any).
   It/those is/are an array of:  */
typedef struct tagRGBQUAD {
  BYTE    rgbBlue;
  BYTE    rgbGreen;
  BYTE    rgbRed;
  BYTE    rgbReserved;
} RGBQUAD;
/* Bitmasks specify which bits are occupied by what colors
   in 16 and 32 bit bitmaps. Three RBGQUADs specify the red, green, and
   blue bitmasks respectively.  */

/* Last would come the bitmap bits (palette indices or literal RGB
   values).  */

#pragma pack(pop)
/* #pragma ms-struct(pop) */
