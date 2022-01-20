#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char* get_buffered_file(FILE**);
void free_buffered_file(unsigned char**);

int validate_bitmap_file(FILE*);
void read_and_print_bitmapInfo(unsigned char*);

void read_and_print_bitmapcoreheader(unsigned char*);
void read_and_print_bitmapinfoheader(unsigned char*);
void read_and_print_bitmapv4header(unsigned char*);
void read_and_print_bitmapv5header(unsigned char*);

void biCompression_to_sotring(unsigned char*, size_t, DWORD);
void bv4v4Compression_to_string(unsigned char*, size_t, DWORD);
void bv4CsType_to_string(unsigned char*, size_t, DWORD);
void bv5Compression_to_string(unsigned char*, size_t, DWORD);
void bv5Cstype_to_string(unsigned char*, size_t, DWORD);
void bv5Intent_to_string(unsigned char*, size_t, DWORD);

int main(int argc, char* argv[])
{
    if (argc < 2)  {
        printf_s("Failed to provide filename!\r\n"); return -1;
    }
    FILE* fbitmap;
    if (0 != fopen_s(&fbitmap, argv[1], "rb")) {
        printf_s("Failed to find/open file!\r\n"); return -1;
    }
    if (!validate_bitmap_file(fbitmap)) {
        fclose(fbitmap);
         printf_s("Not a valid bitmap file!\r\n");
        return -1;
    }
    unsigned char* fbuffer = get_buffered_file(&fbitmap);
    if (!fbuffer) {
        printf_s("Failed to read/allocate memory!\r\n");
        return -1;
    }
    read_and_print_bitmapInfo(fbuffer);
    free_buffered_file(&fbuffer);
}

void biCompression_to_string(unsigned char* buffer, size_t maxbufferSize, DWORD dwFlags)
{
    ZeroMemory(buffer, maxbufferSize);
    if (0 == dwFlags) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "FLAGS_NONE");
        return;
    }
    if (dwFlags & BI_RGB) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "BI_RGB, ");
    }
    if (dwFlags & BI_BITFIELDS) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "BI_BITFIELDS");
    }
}

void bv4v4Compression_to_string(unsigned char* buffer, size_t maxbufferSize, DWORD dwFlags)
{
    ZeroMemory(buffer, maxbufferSize);
    if (0 == dwFlags) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "FLAGS_NONE");
        return;
    }
    if (dwFlags & BI_RGB) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "BI_RGB, ");
    }
    if (dwFlags & BI_RLE8) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "BI_RLE8, ");
    }
    if (dwFlags & BI_RLE4) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "BI_RLE4, ");
    }
    if (dwFlags & BI_BITFIELDS) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "BI_BITFIELDS, ");
    }
    if (dwFlags & BI_JPEG) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "BI_JPEG, ");
    }
    if (dwFlags & BI_PNG) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "BI_PNG, ");
    }
}
void bv4CsType_to_string(unsigned char* buffer, size_t maxbufferSize, DWORD dwFlags)
{
    ZeroMemory(buffer, sizeof(maxbufferSize));
    if (0 == dwFlags) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "FLAGS_NONE");
        return;
    }
    if (dwFlags & LCS_CALIBRATED_RGB) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "LCS_CALIBRATED_RGB");
    }
}

void bv5Compression_to_string(unsigned char* buffer, size_t maxbufferSize, DWORD dwFlags)
{
    if (0 == dwFlags) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "FLAGS_NONE");
        return;
    }
    bv4v4Compression_to_string(buffer, maxbufferSize, dwFlags);
}

void bv5CsType_to_string(unsigned char* buffer, size_t maxbufferSize, DWORD dwFlags)
{
    if (0 == dwFlags) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "FLAGS_NONE");
        return;
    }
    bv4CsType_to_string(buffer, maxbufferSize, dwFlags);
    if (dwFlags & LCS_sRGB) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "LCS_sRGB, ");
    }
    if (dwFlags & LCS_WINDOWS_COLOR_SPACE) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "LCS_WINDOWS_COLOR_SPACE, ");
    }
    if (dwFlags & PROFILE_LINKED) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "PROFILE_LINKED, ");
    }
    if (dwFlags & PROFILE_EMBEDDED) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "PROFILE_EMBEDDED");
    }
}

void bv5Intent_to_string(unsigned char* buffer, size_t maxbufferSize, DWORD dwFlags)
{
    if (0 == dwFlags) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "FLAGS_NONE");
        return;
    }
    if (dwFlags & LCS_GM_ABS_COLORIMETRIC) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "LCS_GM_ABS_COLORIMETRIC, ");
    }
    if (dwFlags & LCS_GM_BUSINESS) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "LCS_GM_BUSINESS, ");
    }
    if (dwFlags & LCS_GM_GRAPHICS) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "LCS_GM_GRAPHICS, ");
    }
    if (dwFlags & LCS_GM_IMAGES) {
        if (strnlen_s(buffer, maxbufferSize) == maxbufferSize) return;
        strcat_s(buffer, maxbufferSize, "LCS_GM_IMAGES");
    }
}


void read_and_print_bitmapv5header(unsigned char* fbuffer)
{
    BITMAPV5HEADER* header = (BITMAPV5HEADER*)(fbuffer);
    CIEXYZTRIPLE* bV5Endpoints = &(header->bV5Endpoints);
    CIEXYZ* red = &(bV5Endpoints->ciexyzRed), *blue = &(bV5Endpoints->ciexyzGreen),
        *green = &(bV5Endpoints->ciexyzBlue);
    unsigned char lpszbv5Compression[50], lpszbv5Cstype[50], lpszbv5Intent[50];

    bv5Compression_to_string(lpszbv5Compression, sizeof(lpszbv5Compression), header->bV5Compression);
    bv5CsType_to_string(lpszbv5Cstype, sizeof(lpszbv5Cstype), header->bV5CSType);
    bv5Intent_to_string(lpszbv5Intent, sizeof(lpszbv5Intent), header->bV5Intent);

    printf_s("\r\nType: BITMAPV5HEADER\r\n\r\n");
    printf_s("bV5Size:\t\t%u\r\n"
        "bV5Width:\t\t%u\r\n"
        "bV5Height:\t\t%u\r\n"
        "bV5Planes:\t\t%u\r\n"
        "bV5BitCount:\t\t%u\r\n"
        "bV5Compression:\t\t%s\r\n"
        "bV5SizeImage:\t\t%u\r\n"
        "bV5XPelsPerMeter:\t%u\r\n"
        "bV5YPelsPerMeter:\t%u\r\n"
        "bV5ClrUsed:\t\t%u\r\n"
        "bV5ClrImportant:\t%u\r\n"
        "bV5RedMask:\t\t%#010x\r\n"
        "bV5GreenMask:\t\t%#010x\r\n"
        "bV5BlueMask:\t\t%#010x\r\n"
        "bV5AlphaMask:\t\t%#010x\r\n"
        "bV5CSType:\t\t%s\r\n"
        "bV5Endpoints:\t\tRed - (%u, %u, %u)"
        "\t\tGreen - (%u, %u, %u)\tBlue - (%u, %u, %u)\r\n"
        "bV5GammaRed:\t\t%u\r\n"
        "bV5GammaGreen:\t\t%u\r\n"
        "bV5GammaBlue:\t\t%u\r\n"
        "bv5Intent:\t\t%s\r\n"
        "bV5ProfileData:\t\t%u\r\n"
        "bV5ProfileSize:\t\t%u\r\n"
        "bV5Reserved:\t\t%u\r\n", header->bV5Size, header->bV5Width,
        header->bV5Height, header->bV5Planes, header->bV5BitCount,
        lpszbv5Compression, header->bV5SizeImage,
        header->bV5XPelsPerMeter, header->bV5YPelsPerMeter,
        header->bV5ClrUsed, header->bV5ClrImportant, header->bV5RedMask,
        header->bV5GreenMask, header->bV5BlueMask, header->bV5AlphaMask,
        lpszbv5Cstype, red->ciexyzX, red->ciexyzY,
        red->ciexyzZ, green->ciexyzX, green->ciexyzY,
        green->ciexyzZ, blue->ciexyzX, blue->ciexyzY,
        blue->ciexyzZ, header->bV5GammaRed, header->bV5GammaGreen,
        header->bV5GammaBlue, lpszbv5Intent, header->bV5ProfileData, header->bV5ProfileSize,
        header->bV5Reserved);
}

void read_and_print_bitmapv4header(unsigned char* fbuffer)
{
    BITMAPV4HEADER* header = (BITMAPV4HEADER*)(fbuffer);
    CIEXYZTRIPLE* bv4Endpoints = &(header->bV4Endpoints);
    CIEXYZ* red = &(bv4Endpoints->ciexyzRed), *blue = &(bv4Endpoints->ciexyzGreen),
        *green = &(bv4Endpoints->ciexyzBlue);
    unsigned char lpszbv4v4Compression[50], lpszbv4CsType[50];

    bv4v4Compression_to_string(lpszbv4v4Compression, sizeof(lpszbv4v4Compression), header->bV4V4Compression);
    bv4CsType_to_string(lpszbv4CsType, sizeof(lpszbv4CsType), header->bV4CSType);

    printf_s("\r\nType: BITMAPV4HEADER\r\n");
    printf_s("bV4Size:\t\t%u\r\n"
        "bV4Width:\t\t%u\r\n"
        "bV4Height:\t\t%u\r\n"
        "bV4Planes:\t\t%u\r\n"
        "bV4BitCount:\t\t%u\r\n"
        "bV4V4Compression:\t\t%s\r\n"
        "bV4SizeImage:\t\t%u\r\n"
        "bV4XPelsPerMeter:\t%u\r\n"
        "bV4YPelsPerMeter:\t%u\r\n"
        "bV4ClrUsed:\t\t%u\r\n"
        "bV4ClrImportant:\t%u\r\n"
        "bV4RedMask:\t\t%#010x\r\n"
        "bV4GreenMask:\t\t%#010x\r\n"
        "bV4BlueMask:\t\t%#010x\r\n"
        "bV4AlphaMask:\t\t%#010x\r\n"
        "bV4CSType:\t\t%s\r\n"
        "bV4Endpoints:\t\tRed - (%u, %u, %u)"
        "\tGreen - (%u, %u, %u)\tBlue - (%u, %u, %u)\r\n"
        "bV4GammaRed:\t\t%u\r\n"
        "bV4GammaGreen:\t\t%u\r\n"
        "bV4GammaBlue:\t\t%u\r\n", header->bV4Size, header->bV4Width,
        header->bV4Height, header->bV4Planes, header->bV4BitCount,
        lpszbv4v4Compression, header->bV4SizeImage,
        header->bV4XPelsPerMeter, header->bV4YPelsPerMeter,
        header->bV4ClrUsed, header->bV4ClrImportant, header->bV4RedMask,
        header->bV4GreenMask, header->bV4BlueMask, header->bV4AlphaMask,
        lpszbv4CsType, red->ciexyzX, red->ciexyzY,
        red->ciexyzZ,green->ciexyzX, green->ciexyzY,
        green->ciexyzZ, blue->ciexyzX, blue->ciexyzY,
        blue->ciexyzZ, header->bV4GammaRed, header->bV4GammaGreen,
        header->bV4GammaBlue);
}

void read_and_print_bitmapinfoheader(unsigned char* fbuffer)
{
     BITMAPINFOHEADER* header = (BITMAPINFOHEADER*)(fbuffer);
    unsigned char lpszbiCompression[50];

    biCompression_to_string(lpszbiCompression, sizeof(lpszbiCompression), header->biCompression);

    printf_s("\r\nType: BITMAPINFOHEADER\r\n");
    printf_s("biSize:\t\t\t%u\r\n"
        "bWidth:\t\t\t%u\r\n"
        "biHeight:\t\t%u\r\n"
        "biPlanes:\t\t%u\r\n"
        "biBitCount:\t\t%u\r\n"
        "biCompression:\t\t%s\r\n"
        "biSizeImage:\t\t%u\r\n"
        "biXPelsPerMeter:\t%u\r\n"
        "biYPelsPerMeter:\t%u\r\n"
        "biClrUsed:\t\t%u\r\n"
        "biClrImportant:\t\t%u\r\n", header->biSize, header->biWidth,
        header->biHeight, header->biPlanes, header->biBitCount,
        lpszbiCompression, header->biSizeImage, header->biXPelsPerMeter,
        header->biYPelsPerMeter, header->biClrUsed, header->biClrImportant);
}

void read_and_print_bitmapcoreheader(unsigned char* fbuffer)
{
    printf_s("Type: BITMAPCOREHEADER\r\n");
    BITMAPCOREHEADER* header = (BITMAPCOREHEADER*)(fbuffer);
    printf_s("bcSize: %u\r\n"
        "bcWidth: %u\r\n"
        "bcHeight: %u\r\n"
        "bcPlanes: %u\r\n"
        "bcBitCount, %u\r\n", header->bcSize, header->bcWidth,
        header->bcHeight,header->bcPlanes, header->bcBitCount);
}

void read_and_print_bitmapInfo(unsigned char* fbuffer)
{
    DWORD headerSize  = *((DWORD*)(fbuffer));

    switch (headerSize) {
        case 12: read_and_print_bitmapcoreheader(fbuffer);
            break;
        case 40: read_and_print_bitmapinfoheader(fbuffer);
            break;
        case 108: read_and_print_bitmapv4header(fbuffer);
            break;
        case 124: read_and_print_bitmapv5header(fbuffer);
            break;
        default: printf_s("bitmap not recognized!\r\n");
            break;
    }
}

int validate_bitmap_file(FILE* file)
{
    unsigned char BM[2];
    if (2 != fread_s(BM, sizeof(BM), 1, 2, file)) {
        return 0;
    }
    if (0 != fseek(file, 0, SEEK_SET)) {
        return 0;
    }
    return ('B' == BM[0] && 'M' == BM[1]);
}

unsigned char* get_buffered_file(FILE** pfile)
{
    DWORD filesztoRead;
    if (0 != fseek(*pfile, 0xE, SEEK_SET)) {
        fclose(*pfile);
        return NULL;
    }
    if (1 != fread_s(&filesztoRead, sizeof(filesztoRead),
         sizeof(filesztoRead), 1, *pfile))
    {
        fclose(*pfile);
        return NULL;
    }
    unsigned char* pbuf = (unsigned char*)malloc((size_t)filesztoRead);
    if (!pbuf) {
        fclose(*pfile);
        return NULL;
    }
    if (0 != fseek(*pfile, 0xE, SEEK_SET)) {
        fclose(*pfile);
        return NULL;
    }
    if ((size_t)filesztoRead != fread_s(pbuf, (size_t)filesztoRead, 1, 
        (size_t)filesztoRead, *pfile))
    {
        free(*pfile);
        fclose(*pfile);
        return NULL;
    }
    fclose(*pfile);
    *pfile = NULL;
    return pbuf;
}

void free_buffered_file(unsigned char** ppbuf)
{
    free(*ppbuf);
    *ppbuf = NULL;
}
