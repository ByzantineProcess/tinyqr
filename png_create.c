/*	PNG File Generator
*	by B.J. Guillot (bguillot@acm.org)
*	Copyright (C) 2016
*	All Rights Reserved
*	Released under MIT License
*	Version 1.0
*	2016-07-01
*/

#include <stdio.h>	//for fopen, fclose, FILE
#include <stdint.h>	//for uint32_t, etc.
#include <stdlib.h>	//for malloc,free
#include <string.h>	//for memcpy
#include <windows.h>	//for BITMAPFILEHEADER, BITMAPINFOHEADER

// -----------------------------------------------------------------------------------------------

void save_bitmap(const char* filename, unsigned char** image, uint32_t width, uint32_t height) {
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    FILE* file;

    uint32_t borderWidth = 2;
    uint32_t newWidth = width + 2 * borderWidth;
    uint32_t newHeight = height + 2 * borderWidth;

    uint32_t rowSize = (newWidth * 3 + 3) & ~3;
    uint32_t dataSize = rowSize * newHeight;

    memset(&fileHeader, 0, sizeof(fileHeader));
    memset(&infoHeader, 0, sizeof(infoHeader));

    fileHeader.bfType = 0x4D42; 
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + dataSize;

    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = newWidth;
    infoHeader.biHeight = newHeight;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = dataSize;

    file = fopen(filename, "wb");
    if (!file) return;

    fwrite(&fileHeader, 1, sizeof(fileHeader), file);
    fwrite(&infoHeader, 1, sizeof(infoHeader), file);

    unsigned char* rowBuffer = (unsigned char*)calloc(1, rowSize);
    if (!rowBuffer) { fclose(file); return; }

    for (int y = newHeight - 1; y >= 0; y--) {
        int idx = 0;
        for (int x = 0; x < newWidth; x++) {
            unsigned char val = 255;
            if (y >= (int)borderWidth && y < (int)(height + borderWidth) &&
                x >= (int)borderWidth && x < (int)(width + borderWidth)) {
                val = image[y - borderWidth][x - borderWidth];
            }
            rowBuffer[idx++] = val;
            rowBuffer[idx++] = val;
            rowBuffer[idx++] = val;
        }
        fwrite(rowBuffer, 1, rowSize, file);
    }

    free(rowBuffer);
    fclose(file);
}


void img_create(uint32_t width, uint32_t height, unsigned char **image, char* filename, uint32_t border) {
    if (image == NULL) {
        return;
    }

    // Save the bitmap using GDI
    save_bitmap(filename, image, width, height);


}