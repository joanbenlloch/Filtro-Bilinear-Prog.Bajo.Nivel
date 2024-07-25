// Joan Benlloch García y Gonzalo de Antonio Sierra

#include "bmpP1.h"
#include <iterator>
#include <vector>
#include <cstring>

BMPReader::BMPReader(string fileName)
{
    // Open file
    ifstream fIn(fileName);
    char *byteFilePointer = NULL;

    // Read bmp header
    std::vector<char> *bytes = new std::vector<char>(std::istreambuf_iterator<char>(fIn), {});

    byteFilePointer = bytes->data();
    bmpHeader = ((bmpHeader_t *)(byteFilePointer))[0];

    byteFilePointer += sizeof(bmpHeader_t);

    uint32_t bmpType = ((uint32_t *)(byteFilePointer))[0];

    dibHeader.bmpType = bmpType;

    std::cout << "El tipo de bmp es: " << bmpType << endl;

    switch (bmpType)
    {
    case 12:
    {
        dibHeader.dibHeader12 = ((dibHeader12_t *)(byteFilePointer))[0];
        byteFilePointer = &(bytes->data()[bmpHeader.dataOffset]);
        if (dibHeader.dibHeader12.bpp != 24)
        {
            std::cout << "El formato bpp no soportado: " << dibHeader.dibHeader12.bpp << endl;
        }
        else
        {
            int numPixels = dibHeader.dibHeader12.width * dibHeader.dibHeader12.height;
            rawData.resize(numPixels);
            memcpy(rawData.data(), byteFilePointer, sizeof(pixel24b_t) * numPixels);
            std::cout << "Fichero cargado" << endl;
        }
        break;
    }
    case 40:
    {
        dibHeader.dibHeader40 = ((dibHeader40_t *)(byteFilePointer))[0];
        byteFilePointer = &(bytes->data()[bmpHeader.dataOffset]);
        if (dibHeader.dibHeader40.bpp != 24)
        {
            std::cout << "El formato bpp no soportado: " << dibHeader.dibHeader40.bpp << endl;
        }
        else
        {
            int numPixels = dibHeader.dibHeader40.width * dibHeader.dibHeader40.height;
            rawData.resize(numPixels);
            memcpy(rawData.data(), byteFilePointer, sizeof(pixel24b_t) * numPixels);
            std::cout << "Fichero cargado" << endl;
        }
        break;
    }
    case 64:
        dibHeader.dibHeader64 = ((dibHeader64_t *)(byteFilePointer))[0];
        byteFilePointer = &(bytes->data()[bmpHeader.dataOffset]);
        if (dibHeader.dibHeader64.bpp != 24)
        {
            std::cout << "El formato bpp no soportado: " << dibHeader.dibHeader64.bpp << endl;
        }
        else
        {
            int numPixels = dibHeader.dibHeader64.width * dibHeader.dibHeader64.height;
            rawData.resize(numPixels);
            memcpy(rawData.data(), byteFilePointer, sizeof(pixel24b_t) * numPixels);
            std::cout << "Fichero cargado" << endl;
        }
        break;
    case 16:
        dibHeader.dibHeader16 = ((dibHeader16_t *)(byteFilePointer))[0];
        byteFilePointer = &(bytes->data()[bmpHeader.dataOffset]);
        if (dibHeader.dibHeader16.bpp != 24)
        {
            std::cout << "El formato bpp no soportado: " << dibHeader.dibHeader16.bpp << endl;
        }
        else
        {
            int numPixels = dibHeader.dibHeader16.width * dibHeader.dibHeader16.height;
            rawData.resize(numPixels);
            memcpy(rawData.data(), byteFilePointer, sizeof(pixel24b_t) * numPixels);
            std::cout << "Fichero cargado" << endl;
        }
        break;
    case 52:
        dibHeader.dibHeader52 = ((dibHeader52_t *)(byteFilePointer))[0];
        byteFilePointer = &(bytes->data()[bmpHeader.dataOffset]);
        if (dibHeader.dibHeader52.bpp != 24)
        {
            std::cout << "El formato bpp no soportado: " << dibHeader.dibHeader52.bpp << endl;
        }
        else
        {
            int numPixels = dibHeader.dibHeader52.width * dibHeader.dibHeader52.height;
            rawData.resize(numPixels);
            memcpy(rawData.data(), byteFilePointer, sizeof(pixel24b_t) * numPixels);
            std::cout << "Fichero cargado" << endl;
        }
        break;
    case 56:
        dibHeader.dibHeader56 = ((dibHeader56_t *)(byteFilePointer))[0];
        byteFilePointer = &(bytes->data()[bmpHeader.dataOffset]);
        if (dibHeader.dibHeader56.bpp != 24)
        {
            std::cout << "El formato bpp no soportado: " << dibHeader.dibHeader56.bpp << endl;
        }
        else
        {
            int numPixels = dibHeader.dibHeader56.width * dibHeader.dibHeader56.height;
            rawData.resize(numPixels);
            memcpy(rawData.data(), byteFilePointer, sizeof(pixel24b_t) * numPixels);
            std::cout << "Fichero cargado" << endl;
        }
        break;
    case 108:
        dibHeader.dibHeader108 = ((dibHeader108_t *)(byteFilePointer))[0];
        byteFilePointer = &(bytes->data()[bmpHeader.dataOffset]);
        if (dibHeader.dibHeader108.bpp != 24)
        {
            std::cout << "El formato bpp no soportado: " << dibHeader.dibHeader108.bpp << endl;
        }
        else
        {
            int numPixels = dibHeader.dibHeader108.width * dibHeader.dibHeader108.height;
            rawData.resize(numPixels);
            memcpy(rawData.data(), byteFilePointer, sizeof(pixel24b_t) * numPixels);
            std::cout << "Fichero cargado" << endl;
        }
        break;
    case 124:
        dibHeader.dibHeader124 = ((dibHeader124_t *)(byteFilePointer))[0];
        byteFilePointer = &(bytes->data()[bmpHeader.dataOffset]);
        if (dibHeader.dibHeader124.bpp != 24)
        {
            std::cout << "El formato bpp no soportado: " << dibHeader.dibHeader124.bpp << endl;
        }
        else
        {
            int numPixels = dibHeader.dibHeader124.width * dibHeader.dibHeader124.height;
            rawData.resize(numPixels);
            memcpy(rawData.data(), byteFilePointer, sizeof(pixel24b_t) * numPixels);
            std::cout << "Fichero cargado" << endl;
        }
        break;
    default:
        cerr << "Error: Tipo de encabezado DIB no soportado" << endl;
        break;
    }

    delete bytes;
    fIn.close();
}

float *BMPReader::getRAWData()
{
    float *data = new float[rawData.size() * 3];
    for (int i = 0; i < rawData.size(); i++)
    {
        data[i * 3] = rawData[i].r;
        data[i * 3 + 1] = rawData[i].g;
        data[i * 3 + 2] = rawData[i].b;
    }
    return data;
}

void BMPReader::setRAWData(float *data)
{
    for (int i = 0; i < rawData.size(); i++)
    {
        rawData[i].r = (unsigned char)data[i * 3];
        rawData[i].g = (unsigned char)data[i * 3 + 1];
        rawData[i].b = (unsigned char)data[i * 3 + 2];
    }
}

void BMPReader::saveBMP(string fileNameOut)
{
    switch (dibHeader.bmpType)
    {
    case 12:
    {
        ofstream fOut(fileNameOut, ios::binary);
        fOut.write((char *)&bmpHeader, sizeof(bmpHeader_t));
        fOut.write((char *)&dibHeader.dibHeader12, sizeof(dibHeader12_t));
        fOut.write((char *)rawData.data(), rawData.size() * sizeof(pixel24b_t));
        fOut.close();
        break;
    }
    case 40:
    {
        ofstream fOut(fileNameOut, ios::binary);
        fOut.write((char *)&bmpHeader, sizeof(bmpHeader_t));
        fOut.write((char *)&dibHeader.dibHeader40, sizeof(dibHeader40_t));
        fOut.write((char *)rawData.data(), rawData.size() * sizeof(pixel24b_t));
        fOut.close();
        break;
    }
    case 64:
    {
        ofstream fOut(fileNameOut, ios::binary);
        fOut.write((char *)&bmpHeader, sizeof(bmpHeader_t));
        fOut.write((char *)&dibHeader.dibHeader64, sizeof(dibHeader64_t));
        fOut.write((char *)rawData.data(), rawData.size() * sizeof(pixel24b_t));
        fOut.close();
        break;
    }
    case 16:
    {
        ofstream fOut(fileNameOut, ios::binary);
        fOut.write((char *)&bmpHeader, sizeof(bmpHeader_t));
        fOut.write((char *)&dibHeader.dibHeader16, sizeof(dibHeader16_t));
        fOut.write((char *)rawData.data(), rawData.size() * sizeof(pixel24b_t));
        fOut.close();
        break;
    }
    case 52:
    {
        ofstream fOut(fileNameOut, ios::binary);
        fOut.write((char *)&bmpHeader, sizeof(bmpHeader_t));
        fOut.write((char *)&dibHeader.dibHeader52, sizeof(dibHeader52_t));
        fOut.write((char *)rawData.data(), rawData.size() * sizeof(pixel24b_t));
        fOut.close();
        break;
    }
    case 56:
    {
        ofstream fOut(fileNameOut, ios::binary);
        fOut.write((char *)&bmpHeader, sizeof(bmpHeader_t));
        fOut.write((char *)&dibHeader.dibHeader56, sizeof(dibHeader56_t));
        fOut.write((char *)rawData.data(), rawData.size() * sizeof(pixel24b_t));
        fOut.close();
        break;
    }
    case 108:
    {
        ofstream fOut(fileNameOut, ios::binary);
        fOut.write((char *)&bmpHeader, sizeof(bmpHeader_t));
        fOut.write((char *)&dibHeader.dibHeader108, sizeof(dibHeader108_t));
        fOut.write((char *)rawData.data(), rawData.size() * sizeof(pixel24b_t));
        fOut.close();
        break;
    }
    case 124:
    {
        ofstream fOut(fileNameOut, ios::binary);
        fOut.write((char *)&bmpHeader, sizeof(bmpHeader_t));
        fOut.write((char *)&dibHeader.dibHeader124, sizeof(dibHeader124_t));
        fOut.write((char *)rawData.data(), rawData.size() * sizeof(pixel24b_t));
        fOut.close();
        break;
    }
    default:
        cerr << "Error: Tipo de encabezado DIB no soportado" << endl;
        break;
    }
}

uint64_t BMPReader::getWidth()
{
    switch (dibHeader.bmpType)
    {
    case 12:
        return dibHeader.dibHeader12.width;
    case 40:
        return dibHeader.dibHeader40.width;
    case 64:
        return dibHeader.dibHeader64.width;
    case 16:
        return dibHeader.dibHeader16.width;
    case 52:
        return dibHeader.dibHeader52.width;
    case 56:
        return dibHeader.dibHeader56.width;
    case 108:
        return dibHeader.dibHeader108.width;
    case 124:
        return dibHeader.dibHeader124.width;
    default:
        cerr << "Error: Tipo de encabezado DIB no soportado" << endl;
        return 0;
    }
}

uint64_t BMPReader::getHeight()
{
    switch (dibHeader.bmpType)
    {
    case 12:
        return dibHeader.dibHeader12.height;
    case 40:
        return dibHeader.dibHeader40.height;
    case 64:
        return dibHeader.dibHeader64.height;
    case 16:
        return dibHeader.dibHeader16.height;
    case 52:
        return dibHeader.dibHeader52.height;
    case 56:
        return dibHeader.dibHeader56.height;
    case 108:
        return dibHeader.dibHeader108.height;
    case 124:
        return dibHeader.dibHeader124.height;
    default:
        cerr << "Error: Tipo de encabezado DIB no soportado" << endl;
        return 0;
    }
}

uint64_t BMPReader::getBPP()
{
    switch (dibHeader.bmpType)
    {
    case 12:
        return dibHeader.dibHeader12.bpp;
    case 40:
        return dibHeader.dibHeader40.bpp;
    case 64:
        return dibHeader.dibHeader64.bpp;
    case 16:
        return dibHeader.dibHeader16.bpp;
    case 52:
        return dibHeader.dibHeader52.bpp;
    case 56:
        return dibHeader.dibHeader56.bpp;
    case 108:
        return dibHeader.dibHeader108.bpp;
    case 124:
        return dibHeader.dibHeader124.bpp;
    default:
        cerr << "Error: Tipo de encabezado DIB no soportado" << endl;
        return 0;
    }
}