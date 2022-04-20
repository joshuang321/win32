#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned char *LPSZ_WHENCE[] = { "SEEK_SET", "SEEK_CUR", "SEEK_END" };
const unsigned char LPSZ_DEFAULT_NAME[] = "ico_output.ico";

#pragma pack(push, 1)
typedef struct tagICONDIR
{
    unsigned short icoReserved, icoImageType, icoNumImages;
} ICONDIR;
typedef struct tagICONDIRENTRY
{
    unsigned char biWidth, biHeight, biClrCount, biReserved;
    unsigned short biPlanes, biBitCount;
    unsigned int biSize, biOffset;
} ICONDIRENTRY;
typedef struct tagBITMAPCOREHEADER
{
    unsigned int bcSize;
    unsigned short bcWidth, bcHeight, bcPlanes, bcBitCount;
} BITMAPCOREHEADER;
typedef struct tagBITMAPINFOHEADER
{
    unsigned int biSize, biWidth, biHeight;
    unsigned short biPlanes, biBitCount;
    unsigned int biCompression, biSizeImage, biXPelsPerMeter, biYPelsPerMeter,
        biClrUsed, biClrImportant;
} BITMAPINFOHEADER;
typedef struct tagCIEXYZ
{
    unsigned int ciexyzX, ciexyzY, ciexyzZ;
} CIEXYZ;
typedef struct tagCIEXYZTRIPLE
{
    CIEXYZ ciexyzRed, ciexyzGreen, ciexyzBlue;
} CIEXYZTRIPLE;
typedef struct tagBITMAPV4HEADER
{
    unsigned int bV4Size, bV4Width, bV4Height;
    unsigned short bV4Planes, bV4BitCount;
    unsigned int bV4V4Compression, bV4SizeImage, bV4XPelsPerMeter, bV4YPelsPerMeter,
        bV4ClrUsed, bV4ClrImportant, bV4RedMask, bV4GreenMask, bV4BlueMask,
        bV4AlphaMask, bV4CSType;
    CIEXYZTRIPLE bV4Endpoints;
    unsigned int bV4GammaRed, bV4GammaGreen, bV4GammaBlue;
} BITMAPV4HEADER;
typedef struct tagBITMAPV5HEADER
{
    unsigned int bV5Size, bV5Width, bV5Height;
    unsigned short bV5Planes, bV5BitCount;
    unsigned int bV5Compression, bV5SizeImage, bV5XPelsPerMeter, bV5YPelsPerMeter,
        bV5ClrUsed, bV5CLrImportant, bV5RedMask, bV5GreenMask, bV5BlueMask,
        bV5AlphaMask, bV5CSType;
    CIEXYZTRIPLE bV5Endpoints;
    unsigned int bV5GammaRed, bV5GammaGreen, bV5GammaBlue, bV5Intent, bV5ProfileData,
       bV5ProfileSize, bV5Reserved;
} BITMAPV5HEADER;
#pragma pack(pop)
typedef union tagBITMAPVARADICHEADER
{
    BITMAPCOREHEADER bitmapcoreHeader; BITMAPINFOHEADER bitmapinfoHeader;
    BITMAPV4HEADER bitmapv4Header; BITMAPV5HEADER bitmapv5Header;
} BITMAPVARADICHEADER;

typedef struct tagBITMAPFILEINFO
{
    FILE* fp;
    unsigned int bixorMaskSize, biandMaskSize;
    union {
        unsigned int biSizeHeader;
        BITMAPVARADICHEADER bitmapvaradicHeader;
    } bitmapheader;
} BITMAPFILEINFO;

#define BITMAPCOREHEADER_SZ 12
#define BITMAPINFOHEADER_SZ 40
#define BITMAPV4HEADER_SZ   108
#define BITMAPV5HEADER_SZ   124

#define SAFE_FWRITE(buffer, size, count, stream) safe_fwrite(buffer, size, count, stream, __LINE__)
#define SAFE_FREAD(buffer, size, count, stream) safe_fread(buffer, size, count, stream, __LINE__)
#define SAFE_FSEEK(stream, offset, whence) safe_fseek(stream, offset, whence, __LINE__)
#define SAFE_FREADANDWRITE(fpsrc, fpdest, size, count, read_offset) safe_freadandwrite(fpsrc, fpdest, size, count, read_offset, __LINE__)

int safe_fwrite(void *buffer, size_t size, size_t count, FILE *stream, int lineno);
int safe_fread(void *buffer, size_t size, size_t count, FILE *stream, int lineno);
int safe_fseek(FILE * stream, long int offset, int whence, int lineno);
int safe_freadandwrite(FILE *fpsrc, FILE* fpdest, size_t size, size_t count, long int read_offset, int lineno);

int main(int argc, char **argv)
{
    unsigned char*          currentargv;
    const unsigned char*    lpszFilename;
    unsigned int            outputIndex = 0,  dwOffset, numImages;
    size_t                  biOffset = 0, biSize;
    FILE*                   fpico;
    BITMAPFILEINFO*         lpbmpfinfo = NULL;
    ICONDIR                 iconDir = { 0, 1, 0 };
    ICONDIRENTRY            iconDirEntry = { 0 };

    if (1 == argc) return -1;
    for (int i=0; i<argc; i++)
    {
        if (!strcmp(argv[i], "-o"))
        {
            if (i == (argc-2)) 
            {
                outputIndex = i+1;
                break;
            }
            else if (i == (argc-1))
            {
                printf_s("Invalid output specified!\r\n");
                return -1;
            }
            else if (i == 1)
            {
                printf_s("No input files specified!\r\n");
                return -1;
            }
            else
            {
                printf_s("Invalid parameter!\r\n");
                return -1;
            }
        }
    }
    iconDir.icoNumImages = ((0 == outputIndex) ? argc : (outputIndex-1))-1;
    biOffset = sizeof(ICONDIR) + iconDir.icoNumImages * sizeof(ICONDIRENTRY);  
    lpbmpfinfo = malloc(iconDir.icoNumImages * sizeof(BITMAPFILEINFO));
    lpszFilename = (0 == outputIndex) ? LPSZ_DEFAULT_NAME : argv[outputIndex];

    if (!lpbmpfinfo)
    {
        printf_s("Failed to allocate memory for bmpfinfo for %d bitmaps\r\n!", iconDir.icoNumImages);
        return -1;
    }
    fopen_s(&fpico, lpszFilename, "wb");
    if (!fpico)
    {
        printf_s("Failed to create/open filename %s, for write!\r\n", lpszFilename);
        return -1;
    }
    SAFE_FWRITE(&iconDir, sizeof(ICONDIR), 1, fpico);
    for (int i=0; i<iconDir.icoNumImages; i++)
    {
        currentargv = argv[i+1];
        fopen_s(&(lpbmpfinfo[i].fp), currentargv, "rb");
        if (lpbmpfinfo[i].fp)
        {
            if (SAFE_FSEEK(lpbmpfinfo[i].fp, 14L, SEEK_SET))
            {
                if (SAFE_FREAD(&(lpbmpfinfo[i].bitmapheader.biSizeHeader), sizeof(unsigned int), 1, lpbmpfinfo[i].fp))
                {
                    if (SAFE_FSEEK(lpbmpfinfo[i].fp, 14L, SEEK_SET))
                    {
                        if (SAFE_FREAD(&(lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader), sizeof(BITMAPVARADICHEADER), 1, lpbmpfinfo[i].fp))
                        {
                            switch (lpbmpfinfo[i].bitmapheader.biSizeHeader)
                            {
                                case BITMAPINFOHEADER_SZ:
                                    iconDirEntry.biWidth = lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapinfoHeader.biWidth;
                                    iconDirEntry.biHeight = lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapinfoHeader.biHeight;
                                    iconDirEntry.biClrCount = 0; iconDirEntry.biReserved = 0;
                                    iconDirEntry.biPlanes = lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapinfoHeader.biPlanes;
                                    iconDirEntry.biBitCount = lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapinfoHeader.biBitCount;

                                    lpbmpfinfo[i].bixorMaskSize = ((int)((iconDirEntry.biBitCount * iconDirEntry.biWidth + 31)/32.0)) * 4 * iconDirEntry.biHeight;
                                    lpbmpfinfo[i].biandMaskSize = ((int)((iconDirEntry.biWidth + 31)/32.0)) * 4 * iconDirEntry.biHeight;
                                    lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapinfoHeader.biSizeImage = lpbmpfinfo[i].bixorMaskSize + lpbmpfinfo[i].biandMaskSize;
                                    iconDirEntry.biSize = (int)sizeof(BITMAPINFOHEADER) + lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapinfoHeader.biSizeImage;
                                    iconDirEntry.biOffset = biOffset;
                                    lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapinfoHeader.biHeight *= 2;
                                    break;
                                case BITMAPCOREHEADER_SZ:
                                    iconDirEntry.biWidth = lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapcoreHeader.bcWidth;
                                    iconDirEntry.biHeight = lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapcoreHeader.bcHeight;
                                    iconDirEntry.biClrCount = 0; iconDirEntry.biReserved = 0;
                                    iconDirEntry.biPlanes = lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapcoreHeader.bcPlanes;
                                    iconDirEntry.biBitCount = lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapcoreHeader.bcBitCount;

                                    lpbmpfinfo[i].bixorMaskSize = ((int)((iconDirEntry.biBitCount * iconDirEntry.biWidth + 31)/32.0)) * 4 * iconDirEntry.biHeight;
                                    lpbmpfinfo[i].biandMaskSize = ((int)((iconDirEntry.biWidth + 31)/32.0)) * 4 * iconDirEntry.biHeight;
                                    iconDirEntry.biSize = (int)sizeof(BITMAPCOREHEADER) + lpbmpfinfo[i].bixorMaskSize + lpbmpfinfo[i].biandMaskSize;
                                    iconDirEntry.biOffset = biOffset;
                                    lpbmpfinfo[i].bitmapheader.bitmapvaradicHeader.bitmapcoreHeader.bcHeight *= 2;
                                    break;
                                case BITMAPV4HEADER_SZ:
                                    break;
                                case BITMAPV5HEADER_SZ:
                                    break;
                            }
                            if (SAFE_FWRITE(&iconDirEntry, sizeof(ICONDIRENTRY), 1, fpico))
                            {
                                biOffset+= iconDirEntry.biSize; continue;
                            }
                        }
                    }
                }
            }
        }
        printf_s("Failed to create %s!\r\n", lpszFilename);
        fclose(fpico); 
        if (!remove(lpszFilename)) printf_s("%s has been removed unsuccessfully!\r\n", lpszFilename);
        for (int j=i; j>-1; j--) fclose(lpbmpfinfo[i].fp);
        free(lpbmpfinfo);
        return -1;
    }
    
    for (int i=0; i<iconDir.icoNumImages; i++)
    {
        currentargv = argv[i+1];
        if (SAFE_FSEEK(lpbmpfinfo[i].fp, 10, SEEK_SET))
        {
            if (SAFE_FREAD(&dwOffset, sizeof(unsigned int), 1, lpbmpfinfo[i].fp))
            {
                if (SAFE_FWRITE(&(lpbmpfinfo[i].bitmapheader), 1, lpbmpfinfo[i].bitmapheader.biSizeHeader, fpico))
                {
                    if (SAFE_FREADANDWRITE(lpbmpfinfo[i].fp, fpico, 1, lpbmpfinfo[i].bixorMaskSize, (long int)dwOffset))
                    {
                        for (int j=0; j<lpbmpfinfo[i].biandMaskSize; j++) putc(0, fpico);
                        continue;
                    }
                }
            }
        }
        printf_s("Failed to create %s!\r\n", lpszFilename);
        printf_s("biSizeHeader: %d\r\n", lpbmpfinfo[i].bitmapheader.biSizeHeader);
        fclose(fpico);
        if(!remove(lpszFilename)) printf_s("%s has been removed successfully!\r\n", lpszFilename);
        free(lpbmpfinfo);
        return -1;
    }
    
    
    fclose(fpico);
    for (int j=0; j<iconDir.icoNumImages; j++) fclose(lpbmpfinfo[j].fp);
    free(lpbmpfinfo);
    printf_s("%s successfully created!\r\n", lpszFilename);

    return 0;
}

int safe_fwrite(void *buffer, size_t size, size_t count, FILE *stream, int lineno)
{
    if (count != fwrite(buffer, size, count, stream))
    {
        printf_s("Failed to do safe fwrite of count %zu, of size %zu on line %d!\r\n",
           count, size, lineno);
        return 0;
    }
    return 1;
}
int safe_fread(void *buffer, size_t size, size_t count, FILE *stream, int lineno)
{
    if (count != fread(buffer, size, count, stream))
    {
        printf_s("Failed to do safe fread of count %zu, of size %zu on line %d\r\n!",
            count, size, lineno);
        return 0;
    }
    return 1;
}
int safe_fseek(FILE* stream, long int offset, int whence, int lineno)
{
    if (fseek(stream, offset, whence))
    {
        printf_s("fseek failed of offset %ld on whence %s on lineno %d!\r\n", offset,
            LPSZ_WHENCE[whence-1], lineno);
        return 0;
    }
    return 1;
}
int safe_freadandwrite(FILE *fpsrc, FILE* fpdest, size_t size, size_t count, long int read_offset,
    int lineno)
{
    long int prevfpos = ftell(fpsrc);
    if (-1L == prevfpos)
    {
        printf_s("Failed to determine fpos of fpsrc on lineno %d!", lineno);
        return 0;
    }
    if (!safe_fseek(fpsrc, read_offset, SEEK_SET, lineno)) return 0;
    void *buffered_io = malloc(size * count);
    if (!buffered_io)
    {
        printf_s("Failed to allocate memory of size %zu on lineno %d", size*count, lineno);
    }
    if (!safe_fread(buffered_io, size, count, fpsrc, lineno)) return 0;
    if (!safe_fwrite(buffered_io, size, count, fpdest, lineno)) return 0;
    if (!safe_fseek(fpsrc, prevfpos, SEEK_SET, lineno)) return 0;
    return 1;
}