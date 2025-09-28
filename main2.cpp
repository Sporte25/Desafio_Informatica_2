#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Rotar bits a la derecha
unsigned char rotarDerecha(unsigned char valor, int n) {
    return (valor >> n) | (valor << (8 - n));
}

// Aplicar XOR
unsigned char aplicarXOR(unsigned char valor, unsigned char clave) {
    return valor ^ clave;
}

// Leer archivo binario
int leerArchivo(const char* nombre, unsigned char*& datos) {
    ifstream archivo(nombre, ios::binary | ios::ate);
    if (!archivo) {
        cout << "Error al abrir " << nombre << endl;
        return 0;
    }

    int tam = archivo.tellg();
    archivo.seekg(0);
    datos = new unsigned char[tam];
    archivo.read((char*)datos, tam);
    archivo.close();
    return tam;
}

// Descompresión tipo LZ78
char* descomprimirLZ78(unsigned char* datos, int tam, int& tamSalida) {
    const int maxEntradas = 1000;
    char* diccionario[maxEntradas];
    int longitudes[maxEntradas];
    int entradas = 0;

    char* salida = new char[tam * 2];
    tamSalida = 0;

    for (int i = 0; i < tam - 2; i += 3) {
        unsigned short indice = datos[i] << 8 | datos[i + 1];
        unsigned char nuevo = datos[i + 2];

        int lenAnt = (indice < entradas) ? longitudes[indice] : 0;
        char* nueva = new char[lenAnt + 1];

        for (int j = 0; j < lenAnt; j++) {
            nueva[j] = diccionario[indice][j];
        }
        nueva[lenAnt] = nuevo;

        diccionario[entradas] = nueva;
        longitudes[entradas] = lenAnt + 1;

        for (int j = 0; j < lenAnt + 1; j++) {
            salida[tamSalida++] = nueva[j];
        }

        entradas++;
    }

    return salida;
}

// Procesar un archivo con n=3, k=90
void procesarArchivo(int numero) {
    string base = "datasetDesarrollo/";
    string nombreArchivo = base + "Encriptado" + to_string(numero) + ".txt";
    string nombreSalida = "mensaje" + to_string(numero) + ".txt";

    cout << "\nProcesando: " << nombreArchivo << endl;

    unsigned char* encriptado = nullptr;
    int tamEncriptado = leerArchivo(nombreArchivo.c_str(), encriptado);
    if (tamEncriptado == 0) return;

    int n = 3;
    unsigned char clave = 90;
    unsigned char* desencriptado = new unsigned char[tamEncriptado];

    for (int i = 0; i < tamEncriptado; i++) {
        unsigned char temp = aplicarXOR(encriptado[i], clave);
        desencriptado[i] = rotarDerecha(temp, n);
    }

    cout << "Inspección manual con n=3, k=90:" << endl;
    for (int i = 0; i < 100 && i < tamEncriptado; i++) {
        cout << (char)desencriptado[i];
    }
    cout << endl;

    // Verificar si está comprimido
    int legibles = 0;
    for (int i = 0; i < 100 && i < tamEncriptado; i++) {
        if (isprint(desencriptado[i])) legibles++;
    }

    ofstream salida(nombreSalida.c_str());

    if (legibles < 80) {
        int tamDescomprimido = 0;
        char* mensaje = descomprimirLZ78(desencriptado, tamEncriptado, tamDescomprimido);

        cout << "Mensaje reconstruido (LZ78):" << endl;
        for (int i = 0; i < tamDescomprimido; i++) {
            cout << mensaje[i];
            salida << mensaje[i];
        }
        cout << endl;

        delete[] mensaje;
    } else {
        cout << "Mensaje desencriptado (texto plano):" << endl;
        for (int i = 0; i < tamEncriptado; i++) {
            cout << (char)desencriptado[i];
            salida << (char)desencriptado[i];
        }
        cout << endl;
    }

    salida.close();
    delete[] desencriptado;
    delete[] encriptado;
}

// Función principal
int main() {
    cout << "Programa iniciado correctamente..." << endl;

    for (int i = 1; i <= 3; i++) {
        procesarArchivo(i);
    }

    return 0;
}