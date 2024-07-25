// Joan Benlloch García y Gonzalo de Antonio Sierra

#pragma once
#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

using namespace std;

typedef struct __attribute__((packed))
{
    uint8_t format[2];
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
} bmpHeader_t;

// 12 BITMAPCOREHEADER
typedef struct __attribute__((packed))
{
    uint32_t dibHeaderSize;
    uint16_t width;
    uint16_t height;
    uint16_t layerCount;
    uint16_t bpp;
} dibHeader12_t;

// 64 OS22XBITMAPHEADER
typedef struct __attribute__((packed))
{
    uint32_t dibHeaderSize;
    uint16_t width;
    uint16_t height;
    uint16_t layerCount;
    uint16_t bpp;
    uint32_t compressMethod;
    uint32_t rawImgSize;
    uint32_t ppmH;
    uint32_t ppmV;
    uint32_t paletteNumColors;
    uint32_t importantNumColors;
} dibHeader64_t;

// 16 OS22XBITMAPHEADER
typedef struct __attribute__((packed))
{
    uint32_t dibHeaderSize;
    uint16_t width;
    uint16_t height;
    uint16_t layerCount;
    uint16_t bpp;
    uint16_t compressMethod;
    uint16_t rawImgSize;
    uint16_t ppmH;
    uint16_t ppmV;
    uint16_t paletteNumColors;
    uint16_t importantNumColors;
} dibHeader16_t;

// 40 BITMAPINFOHEADER
typedef struct __attribute__((packed))
{
    uint32_t dibHeaderSize;
    uint32_t width;
    uint32_t height;
    uint16_t layerCount;
    uint16_t bpp;
    uint32_t compressMethod;
    uint32_t rawImgSize;
    uint32_t ppmH;
    uint32_t ppmV;
    uint32_t paletteNumColors;
    uint32_t importantNumColors;
} dibHeader40_t;

// 52 BITMAPV2INFOHEADER
typedef struct __attribute__((packed))
{
    uint32_t dibHeaderSize;
    uint32_t width;
    uint32_t height;
    uint16_t layerCount;
    uint16_t bpp;
    uint32_t compressMethod;
    uint32_t rawImgSize;
    uint32_t ppmH;
    uint32_t ppmV;
    uint32_t paletteNumColors;
    uint32_t importantNumColors;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
} dibHeader52_t;

// 56 BITMAPV3INFOHEADER
typedef struct __attribute__((packed))
{
    uint32_t dibHeaderSize;
    uint32_t width;
    uint32_t height;
    uint16_t layerCount;
    uint16_t bpp;
    uint32_t compressMethod;
    uint32_t rawImgSize;
    uint32_t ppmH;
    uint32_t ppmV;
    uint32_t paletteNumColors;
    uint32_t importantNumColors;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
} dibHeader56_t;

// 108 BITMAPV4HEADER
typedef struct __attribute__((packed))
{
    uint32_t dibHeaderSize;
    uint32_t width;
    uint32_t height;
    uint16_t layerCount;
    uint16_t bpp;
    uint32_t compressMethod;
    uint32_t rawImgSize;
    uint32_t ppmH;
    uint32_t ppmV;
    uint32_t paletteNumColors;
    uint32_t importantNumColors;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
    uint32_t colorSpaceType;
    uint32_t redX;
    uint32_t redY;
    uint32_t redZ;
    uint32_t greenX;
    uint32_t greenY;
    uint32_t greenZ;
    uint32_t blueX;
    uint32_t blueY;
    uint32_t blueZ;
    uint32_t gammaRed;
    uint32_t gammaGreen;
    uint32_t gammaBlue;
} dibHeader108_t;

// 124 BITMAPV5HEADER
typedef struct __attribute__((packed))
{
    uint32_t dibHeaderSize;
    uint32_t width;
    uint32_t height;
    uint16_t layerCount;
    uint16_t bpp;
    uint32_t compressMethod;
    uint32_t rawImgSize;
    uint32_t ppmH;
    uint32_t ppmV;
    uint32_t paletteNumColors;
    uint32_t importantNumColors;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
    uint32_t colorSpaceType;
    uint32_t redX;
    uint32_t redY;
    uint32_t redZ;
    uint32_t greenX;
    uint32_t greenY;
    uint32_t greenZ;
    uint32_t blueX;
    uint32_t blueY;
    uint32_t blueZ;
    uint32_t gammaRed;
    uint32_t gammaGreen;
    uint32_t gammaBlue;
    uint32_t intent;
    uint32_t profileData;
    uint32_t profileSize;
    uint32_t reserved;
} dibHeader124_t;

// Union
typedef union dibHeader_t
{
    int bmpType;
    dibHeader12_t dibHeader12;
    dibHeader16_t dibHeader16;
    dibHeader40_t dibHeader40;
    dibHeader52_t dibHeader52;
    dibHeader56_t dibHeader56;
    dibHeader64_t dibHeader64;
    dibHeader108_t dibHeader108;
    dibHeader124_t dibHeader124;
} dibHeader_t;

typedef struct pixel24b_t
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel24b_t;


class BMPReader
{

public:
    bmpHeader_t bmpHeader;
    dibHeader_t dibHeader;

    vector<pixel24b_t> rawData;

    BMPReader(string fileName);

    uint32_t getRAWSize() { return bmpHeader.fileSize; };

    uint64_t getWidth();
    uint64_t getHeight();
    uint64_t getBPP();

    float *getRAWData();
    void setRAWData(float *data);

    void saveBMP(string fileNameOut);
    void applyBilinearFilter();
    void applyCrop(int x, int y, int width, int height);
};
