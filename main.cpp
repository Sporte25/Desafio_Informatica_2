//#include <QtCore/QCoreApplication>
#include <stdio.h>
#include <stdlib.h>

// Función para leer un archivo binario y cargarlo en memoria dinámica
unsigned char* leerArchivo(const char* nombreArchivo, int* longitud) {
    FILE* archivo = fopen(nombreArchivo, "rb");
    if (archivo == NULL) {
        printf("Error: No se pudo abrir el archivo: %s\n", nombreArchivo);
        return NULL;
    }

    fseek(archivo, 0, SEEK_END);
    *longitud = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);

    if (*longitud <= 0) {
        printf("Error: El archivo está vacío: %s\n", nombreArchivo);
        fclose(archivo);
        return NULL;
    }

    unsigned char* buffer = new unsigned char[*longitud];
    size_t leidos = fread(buffer, 1, *longitud, archivo);

    if (leidos != *longitud) {
        printf("Error: Lectura incompleta del archivo: %s\n", nombreArchivo);
        delete[] buffer;
        fclose(archivo);
        return NULL;
    }

    fclose(archivo);
    return buffer;
}

// ---------- FUNCIONES AUXILIARES ----------

// Rotar bits a la derecha (inverso de rotación izquierda)
unsigned char rotarDerecha(unsigned char byte, int n) {
    return (byte >> n) | (byte << (8 - n));
}

// Desencriptar mensaje (rotación derecha + XOR inversa)
void desencriptar(unsigned char* entrada, unsigned char* salida, int longitud, int n, unsigned char k) {
    for (int i = 0; i < longitud; ++i) {
        unsigned char x = entrada[i] ^ k;
        salida[i] = rotarDerecha(x, n);
    }
}

// Descompresión tentativa con RLE
int descomprimirRLE(unsigned char* entrada, int longitudEntrada, unsigned char* salida, int tamMaxSalida) {
    int i = 0, j = 0;
    while (i + 1 < longitudEntrada && j < tamMaxSalida) {
        unsigned char caracter = entrada[i];
        unsigned char repeticiones = entrada[i + 1];
        i += 2;

        for (int r = 0; r < repeticiones && j < tamMaxSalida; ++r) {
            salida[j++] = caracter;
        }
    }
    return j;
}

// Búsqueda del fragmento dentro de un texto
bool contieneFragmento(unsigned char* texto, int lenTexto, unsigned char* fragmento, int lenFragmento) {
    for (int i = 0; i <= lenTexto - lenFragmento; ++i) {
        bool coincide = true;
        for (int j = 0; j < lenFragmento; ++j) {
            if (texto[i + j] != fragmento[j]) {
                coincide = false;
                break;
            }
        }
        if (coincide) return true;
    }
    return false;
}

// Descompresión tentativa con LZ78 (simple)
int descomprimirLZ78(unsigned char* entrada, int longitudEntrada, unsigned char* salida, int tamMaxSalida) {
    // Implementación básica para detección, no óptima
    unsigned char diccionario[256][256]; // máximo 256 entradas, 256 chars por entrada
    int longitudes[256]; // longitud de cada entrada
    int numEntradas = 0;

    int i = 0, j = 0;
    while (i < longitudEntrada && j < tamMaxSalida) {
        unsigned char indice = entrada[i++];
        unsigned char siguiente = entrada[i++];

        if (indice == 0) {
            if (j < tamMaxSalida) salida[j++] = siguiente;

            // Guardar nueva entrada
            diccionario[numEntradas][0] = siguiente;
            longitudes[numEntradas] = 1;
            numEntradas++;
        } else if (indice <= numEntradas) {
            int len = longitudes[indice - 1];

            for (int k = 0; k < len && j < tamMaxSalida; ++k) {
                salida[j] = diccionario[indice - 1][k];
                diccionario[numEntradas][k] = salida[j];
                j++;
            }

            if (j < tamMaxSalida) {
                salida[j] = siguiente;
                diccionario[numEntradas][len] = siguiente;
                j++;
            }

            longitudes[numEntradas] = len + 1;
            numEntradas++;
        } else {
            break; // índice inválido
        }
    }

    return j;
}

void desencriptarSoloXOR(unsigned char* entrada, unsigned char* salida, int longitud, unsigned char k) {
    for (int i = 0; i < longitud; ++i) {
        salida[i] = entrada[i] ^ k;
    }
}


int main(int argc, char *argv[])
{
    //QCoreApplication app(argc, argv);

    const char* rutaMensaje = "data/Encriptado1.txt";
    const char* rutaFragmento = "data/pista1.txt";

    int longitudMensaje = 0;
    int longitudFragmento = 0;

    // Leer archivos
    unsigned char* mensaje = leerArchivo(rutaMensaje, &longitudMensaje);
    if (mensaje == NULL) return -1;

    unsigned char* fragmento = leerArchivo(rutaFragmento, &longitudFragmento);
    if (fragmento == NULL) {
        delete[] mensaje;
        return -1;
    }

    // Confirmación
    printf("Archivos cargados y leídos con éxito.\n");

    printf("Analizando compresión...\n");

    bool encontrado = false;

    for (int k = 0; k <= 255 && !encontrado; ++k) {
    unsigned char* salida = new unsigned char[longitudMensaje];
    desencriptarSoloXOR(mensaje, salida, longitudMensaje, (unsigned char)k);

    // Verificar sin descomprimir
    if (contieneFragmento(salida, longitudMensaje, fragmento, longitudFragmento)) {
        printf("El archivo NO está comprimido, y solo se usó XOR como encriptación.\n");
        printf("Clave K = %d\n", k);
        encontrado = true;
    }

    // Verificar RLE
    if (!encontrado) {
        unsigned char* salidaRLE = new unsigned char[50000];
        int lenRLE = descomprimirRLE(salida, longitudMensaje, salidaRLE, 50000);

        if (contieneFragmento(salidaRLE, lenRLE, fragmento, longitudFragmento)) {
            printf("Método de compresión detectado: RLE, y solo se usó XOR.\n");
            printf("Clave K = %d\n", k);
            encontrado = true;
        }

        delete[] salidaRLE;
    }

    // Verificar LZ78
    if (!encontrado) {
        unsigned char* salidaLZ = new unsigned char[50000];
        int lenLZ = descomprimirLZ78(salida, longitudMensaje, salidaLZ, 50000);

        if (contieneFragmento(salidaLZ, lenLZ, fragmento, longitudFragmento)) {
            printf("Método de compresión detectado: LZ78, y solo se usó XOR.\n");
            printf("Clave K = %d\n", k);
            encontrado = true;
        }

        delete[] salidaLZ;
    }

    delete[] salida;
}


    if (!encontrado) {
        printf("No se pudo determinar el método de compresión o el mensaje desencriptado no contiene el fragmento.\n");
    }

    printf("Fragmento en HEX: ");
for (int i = 0; i < longitudFragmento; ++i)
    printf("%02X ", fragmento[i]);
printf("\n");

printf("Fragmento como texto: ");
for (int i = 0; i < longitudFragmento; ++i)
    printf("%c", fragmento[i]);
printf("\n");


printf("Primeros bytes del archivo encriptado:\n");
for (int i = 0; i < 20 && i < longitudMensaje; ++i) {
    printf("%02X ", mensaje[i]);
}
printf("\n");

    


    // Liberar memoria
    delete[] mensaje;
    delete[] fragmento;

    return 0;
}
