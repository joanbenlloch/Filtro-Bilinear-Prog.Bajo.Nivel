// Joan Benlloch García y Gonzalo de Antonio Sierra

#include "bmpP1.h"
#include <thread>
#include <iostream>
#include <cstring>

using namespace std;

void applyBilinearFilterThreads(float *data, int w, int h, float *dataOut, int start, int end)
{

    // Filtro Edge Detection
    
    float filter[3][3] =
        {
            {0.0f, 1.0f, 0.0f},
            {1.0f, -4.0f, 1.0f},
            {0.0f, 1.0f, 0.0f}
        };
    

   // Filtro Sharpen
   /*
   float filter[3][3] =
        {
            {0.0f, -1.0f, 0.0f},
            {-1.0f, 5.0f, -1.0f},
            {0.0f, -1.0f, 0.0f}
        };
    */

   // Filtro Blur
   /*
   float filter[3][3] =
        {
            {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
            {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
            {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f}
        };
    */


    float r = 0, g = 0, b = 0;

    for (int i = start; i < end; i++) {
        for (int j = 0; j < w; j++) {
            r = g = b = 0;

            // Calcular un pixel con los de alrededor
            for (int y = -1; y <= 1; y++) {
                for (int x = -1; x <= 1; x++) {
                    int newY = i + y;
                    int newX = j + x;

                    // Controlamos que el píxel esté dentro de la imagen
                    if (newY >= 0 && newY < h && newX >= 0 && newX < w) {
                        r += data[(newY * w + newX) * 3] *      filter[y + 1][x + 1];
                        g += data[(newY * w + newX) * 3 + 1] *  filter[y + 1][x + 1];
                        b += data[(newY * w + newX) * 3 + 2] *  filter[y + 1][x + 1];
                    }
                }
            }

            // Aplicar la normalización si es necesario
            r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
            g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
            b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

            // Escribir el píxel filtrado en la imagen de salida
            dataOut[(i * w + j) * 3]        = r;
            dataOut[(i * w + j) * 3 + 1]    = g;
            dataOut[(i * w + j) * 3 + 2]    = b;
        }
    }
}

float *increaseSize(float *data, int w, int h)
{
    int oldWidth = w;
    int oldHeight = h;

    // Calcular nuevo ancho y alto con el perímetro adicional
    int newWidth = oldWidth + 2;
    int newHeight = oldHeight + 2;

    // Crear nuevo array para la nueva imagen con el perímetro adicional y rellenar con 0s
    float *newImageData = new float[newWidth * newHeight * 3];
    memset(newImageData, 0, sizeof(float) * newWidth * newHeight * 3);

    // Copiar los píxeles originales al centro de la nueva imagen
    for (int i = 0; i < oldHeight; i++)
        for (int j = 0; j < oldWidth; j++)
            for (int k = 0; k < 3; k++)
                newImageData[((i + 1) * newWidth * 3 + (j + 1) * 3) + k] = data[(i * oldWidth * 3 + j * 3) + k];

    return newImageData;
}

float *reduceSize(float *data, int w, int h)
{
    int oldWidth = w + 2;
    int oldHeight = h + 2;

    // Calcular nuevo ancho y alto con el perímetro adicional
    int newWidth = w;
    int newHeight = h;

    // Crear nuevo array para la nueva imagen con el perímetro adicional y rellenar con 0s
    float *newImageData = new float[newWidth * newHeight * 3];
    memset(newImageData, 0, sizeof(float) * newWidth * newHeight * 3);

    // Copiar los píxeles originales al centro de la nueva imagen
    for (int i = 0; i < newHeight; i++)
        for (int j = 0; j < newWidth; j++)
            for (int k = 0; k < 3; k++)
                newImageData[(i * newWidth * 3 + j * 3) + k] = data[((i + 1) * oldWidth * 3 + (j + 1) * 3) + k];

    return newImageData;
}

int main(int argc, char **argv)
{

    BMPReader *bmp = new BMPReader("TAJ.BMP");
    cout << "El tamanio del fichero es: " << bmp->getRAWSize() << " bytes" << endl;
    cout << "El formato de la imagen es: " << bmp->getWidth()
         << "x" << bmp->getHeight() << " " << bmp->getBPP() << "bits" << endl;
    bmp->saveBMP("COPIA_FLAGB24_P1.BMP");

    float *dataOut = new float[(bmp->getHeight() + 2) * (bmp->getWidth() + 2) * 3];
    for (int i = 0; i < bmp->getHeight() + 2 * bmp->getWidth() + 2 * 3; i++)
        dataOut[i] = 0;

    // Añadir perímetro
    float *dataIncreased = increaseSize(bmp->getRAWData(), bmp->getWidth(), bmp->getHeight());

    int numCPUs = 4;
    int dataSize = bmp->getHeight() + 2;
    int blockSize = dataSize / numCPUs;

    // Parallelize the bilinear filter
    vector<thread *> threads(numCPUs);

    cout << "Applying bilinear filter" << endl;

    for (int i = 0; i < numCPUs; i++)
    {
        int start = i * blockSize;
        int end = (i + 1) * blockSize;
        if (i == numCPUs - 1)
            end = dataSize;
        threads[i] = new thread(applyBilinearFilterThreads, dataIncreased, bmp->getWidth() + 2, bmp->getHeight() + 2, dataOut, start, end);
    }

    cout << "Waiting for threads to finish" << endl;

    // Wait for all threads to finish
    for (int i = 0; i < numCPUs; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    cout << "Bilinear filter applied" << endl;

    float *dataReduced = reduceSize(dataOut, bmp->getWidth(), bmp->getHeight());
    bmp->setRAWData(dataReduced);
    bmp->saveBMP("BILINEAR_FLAGB24_MULTITHREAD.BMP");

    delete bmp;

    return 0;
}