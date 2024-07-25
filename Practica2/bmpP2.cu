// Joan Benlloch García y Gonzalo de Antonio Sierra

#include "bmpP2.h"

#define ANCHO_IMAGEN 800

template<typename T>
class gpuDataManager {
    public:
        T* gpuData;
        T* cpuData;
        int size;
        
    gpuDataManager(int size){
        cudaMalloc(&gpuData, sizeof(T)*size);
        cpuData = new T[size];
        this->size = size;
    }
    
    void copyToGPU(){
        cudaMemcpy(gpuData, cpuData, sizeof(T)*size, cudaMemcpyHostToDevice);
    }
    
    void copyToCPU(){
        cudaMemcpy(cpuData, gpuData, sizeof(T)*size, cudaMemcpyDeviceToHost);
    }
    
    ~gpuDataManager(){
        cudaFree(gpuData);
        delete[] cpuData;
    }
};

__global__ void applyBilinearFilter_k(int width, int height, int filter, float *data, float *dataOut)
{
    float filters[6][3][3] ={
        {
            // Filtro Identidad (Para comprobar si funciona correctamente el acceso a memoria compartida)
            {0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f}
        },
        {
            // Filtro Edge Detection
            {0.0f, 1.0f, 0.0f},
            {1.0f, -4.0f, 1.0f},
            {0.0f, 1.0f, 0.0f}
        },
        {
            // Filtro Sharpen
            {0.0f, -1.0f, 0.0f},
            {-1.0f, 5.0f, -1.0f},
            {0.0f, -1.0f, 0.0f}
        },
        {
            // Filtro Blur
            {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
            {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
            {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f}
        },
        {
            // Filtro Emboss
            {-2.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 1.0f},
            {0.0f, 1.0f, 2.0f}
        },
        {
            // Filtro Outline
            {-1.0f, -1.0f, -1.0f},
            {-1.0f, 8.0f, -1.0f},
            {-1.0f, -1.0f, -1.0f}
        }
    };

    __shared__ float dataShared[3][ANCHO_IMAGEN + 2][3];

    // Columna del pixel
    int columna = threadIdx.x;

    // Fila del pixel
    int fila = blockIdx.x;

    // Indice global del pixel
    int index = fila * width + columna;
    
    dataShared[1][columna][0] = data[index * 3];
    dataShared[1][columna][1] = data[index * 3 + 1];
    dataShared[1][columna][2] = data[index * 3 + 2];

    // Memoria compartida para los píxeles de la fila superior
    int indixFilaSuperior = (fila - 1) * width + columna;
    dataShared[0][columna][0] = data[indixFilaSuperior * 3];
    dataShared[0][columna][1] = data[indixFilaSuperior * 3 + 1];
    dataShared[0][columna][2] = data[indixFilaSuperior * 3 + 2];

    // Memoria compartida para los píxeles de la fila inferior
    int indexFilaInferior = (fila + 1) * width + columna;
    dataShared[2][columna][0] = data[indexFilaInferior * 3];
    dataShared[2][columna][1] = data[indexFilaInferior * 3 + 1];
    dataShared[2][columna][2] = data[indexFilaInferior * 3 + 2];

    // Sincronizar hilos para asegurar que todos los datos estén en la memoria compartida
    __syncthreads();

    if (fila < height && columna < width) {
        float r = 0.0f, g = 0.0f, b = 0.0f;

        // Aplicar filtro bilineal
        for (int dy = 0; dy <= 2; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int x = columna + 1 + dx;

                r += dataShared[dy][x][0] * filters[filter][dy][dx + 1];
                g += dataShared[dy][x][1] * filters[filter][dy][dx + 1];
                b += dataShared[dy][x][2] * filters[filter][dy][dx + 1];
            }
        }

        // Por si salen valores negativos o por encima de 255
        r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
        g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
        b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

        // Escribir píxel en la imagen de salida
        dataOut[index * 3] = r;
        dataOut[index * 3 + 1] = g;
        dataOut[index * 3 + 2] = b;
    }
}


vector<float> increaseSize(vector<float> data, int w, int h)
{
    int oldWidth = w;
    int oldHeight = h;

    // Calcular nuevo ancho y alto con el perímetro adicional
    int newWidth = oldWidth + 2;
    int newHeight = oldHeight + 2;

    // Crear nuevo vector para la nueva imagen con el perímetro adicional y rellenar con 0s
    std::vector<float> newImageData(newWidth * newHeight * 3, 0.0f);

    // Copiar los píxeles originales al centro de la nueva imagen
    for (int i = 0; i < oldHeight; i++)
        for (int j = 0; j < oldWidth; j++)
            for (int k = 0; k < 3; k++)
                newImageData[((i + 1) * newWidth * 3 + (j + 1) * 3) + k] = data[(i * oldWidth * 3 + j * 3) + k];

    return newImageData;
}

vector<float> reduceSize(vector<float> data, int w, int h)
{
    int oldWidth = w + 2;

    // Calcular nuevo ancho y alto con el perímetro adicional
    int newWidth = w;
    int newHeight = h;

    // Crear nuevo vector para la nueva imagen con el perímetro adicional y rellenar con 0s
    std::vector<float> newImageData(newWidth * newHeight * 3, 0.0f);

    // Copiar los píxeles originales al centro de la nueva imagen
    for (int i = 0; i < newHeight; i++)
        for (int j = 0; j < newWidth; j++)
            for (int k = 0; k < 3; k++)
                newImageData[(i * newWidth * 3 + j * 3) + k] = data[((i + 1) * oldWidth * 3 + (j + 1) * 3) + k];

    return newImageData;
}



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

vector<float> BMPReader::getRAWData()
{
    std::vector<float> data(rawData.size() * 3);
    for (int i = 0; i < rawData.size(); i++)
    {
        data[i * 3] = rawData[i].r;
        data[i * 3 + 1] = rawData[i].g;
        data[i * 3 + 2] = rawData[i].b;
    }
    return data;
}

void BMPReader::setRAWData(vector<float> data)
{
    rawData.resize(data.size() / 3);
    for (int i = 0; i < rawData.size(); i++)
    {
        rawData[i].r = data[i * 3];
        rawData[i].g = data[i * 3 + 1];
        rawData[i].b = data[i * 3 + 2];
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

void BMPReader::applyBilinearFilter(int filter)
{
    gpuDataManager<float>* dataRes = new gpuDataManager<float>((getHeight() + 2) * (getWidth() + 2) * 3);
    
    // Añadir perímetro
    vector<float> dataIncreased = increaseSize(getRAWData(), getWidth(), getHeight());
    gpuDataManager<float>* dataToFilter = new gpuDataManager<float>(dataIncreased.size());
    for(int i = 0; i < dataIncreased.size(); i++){
        dataToFilter->cpuData[i] = dataIncreased[i];
    }
    dataToFilter->copyToGPU();

    // Calcular bloques/threads
    int numThreadPerBlock = ANCHO_IMAGEN + 2;
    int numBlock = getHeight() + 2;

    cout << "Applying bilinear filter in CUDA" << endl;

    applyBilinearFilter_k<<<numBlock, numThreadPerBlock>>>(getWidth()+2, getHeight()+2, filter, dataToFilter->gpuData, dataRes->gpuData);

    cout << "Waiting CUDA to finish" << endl;

    // Recuperar resultados de GPU
    dataRes->copyToCPU();
    cudaDeviceSynchronize();

    cout << "Bilinear filter applied in CUDA" << endl;

    vector<float> dataResCPU(dataRes->size);
    for(int i = 0; i < dataResCPU.size(); i++){
        dataResCPU[i] = dataRes->cpuData[i];
    }

    vector<float> dataReduced = reduceSize(dataResCPU, getWidth(), getHeight());
    setRAWData(dataReduced);

    delete dataRes;
    delete dataToFilter;
}