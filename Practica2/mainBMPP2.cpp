// Joan Benlloch García y Gonzalo de Antonio Sierra
// nvcc mainBMPP2.cpp bmpP2.cu -o filter.exe

#include "bmpP2.h"
#include <iostream>
#include <cstring>

using namespace std;

int main(int argc, char **argv)
{
    BMPReader *bmp = new BMPReader("TAJ_800.BMP");
    cout << "El tamanio del fichero es: " << bmp->getRAWSize() << " bytes" << endl;
    cout << "El formato de la imagen es: " << bmp->getWidth()
         << "x" << bmp->getHeight() << " " << bmp->getBPP() << "bits" << endl;
    bmp->saveBMP("TAJ_800.COPIA.BMP");
    int filter = atoi(argv[1]);
    bmp->applyBilinearFilter(filter);
    bmp->saveBMP("TAJ_800.FILTER.CUDA.BMP");

    delete bmp;

    return 0;
}