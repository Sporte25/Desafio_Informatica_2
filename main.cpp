#include <stdio.h>
#include <stdlib.h>

// ======== LECTURA DE ARCHIVO ========
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

// ======== ROTACIÓN Y DESENCRIPTADO ========
unsigned char rotarDerecha(unsigned char byte, int n) {
    return (byte >> n) | (byte << (8 - n));
}

void desencriptar(unsigned char* entrada, unsigned char* salida, int longitud, int n, unsigned char k) {
    for (int i = 0; i < longitud; ++i) {
        unsigned char x = entrada[i] ^ k;
        salida[i] = rotarDerecha(x, n);
    }
}

// ======== DESCOMPRESIÓN RLE ========
int descomprimirRLE(unsigned char* entrada, int longitudEntrada, unsigned char* salida, int tamMaxSalida) {
    int i = 0, j = 0;
    while (i + 2 <= longitudEntrada && j < tamMaxSalida) {
        // Leer 3 bytes: ignorar el primero (aparentemente siempre 00), luego repeticiones y letra
        unsigned char dummy = entrada[i];        // 00 (ignorado)
        unsigned char repeticiones = entrada[i + 1];  // cantidad
        unsigned char caracter = entrada[i + 2]; // letra

        i += 3;

        for (int r = 0; r < repeticiones && j < tamMaxSalida; ++r) {
            salida[j++] = caracter;
        }
    }
    return j;
}

// ======== FUNCIÓN PRINCIPAL ========
int main(int argc, char *argv[])
{
    const char* rutaMensaje = "data/Encriptado1.txt";
    int longitudMensaje = 0;

    // Leer archivo encriptado
    unsigned char* mensaje = leerArchivo(rutaMensaje, &longitudMensaje);
    if (mensaje == NULL) return -1;

    // Parámetros conocidos
    const int rotacion = 3;
    const unsigned char clave = 0x5A;

    // Mostrar configuración
    printf("** Encriptado1.txt **\n");
    printf("Compresion: RLE\n");
    printf("Rotacion: %d\n", rotacion);
    printf("Key= 0x%02X\n", clave);

    // Desencriptar
    unsigned char* desencriptado = new unsigned char[longitudMensaje];
    desencriptar(mensaje, desencriptado, longitudMensaje, rotacion, clave);

    // DEBUG: Ver primeros bytes desencriptados
    printf("\nPrimeros bytes desencriptados (antes de RLE):\n");
    for (int i = 0; i < 20 && i < longitudMensaje; ++i)
        printf("%02X ", desencriptado[i]);
    printf("\n");

    printf("Como texto (puede no ser legible):\n");
    for (int i = 0; i < 20 && i < longitudMensaje; ++i)
        printf("%c", desencriptado[i]);
    printf("\n");

    // Descomprimir
    unsigned char* descomprimido = new unsigned char[50000]; // Tamaño estimado
    int longitudDescomprimido = descomprimirRLE(desencriptado, longitudMensaje, descomprimido, 50000);

    // Mostrar resultado
    printf("\nMensaje final descomprimido y desencriptado:\n");
    for (int i = 0; i < longitudDescomprimido; ++i)
        printf("%c", descomprimido[i]);
    printf("\n");

    // Guardar en archivo
    FILE* archivoSalida = fopen("mensaje1.txt", "wb");
    if (archivoSalida) {
        fwrite(descomprimido, 1, longitudDescomprimido, archivoSalida);
        fclose(archivoSalida);
        printf("\nMensaje guardado en mensaje1.txt\n");
    } else {
        printf("Error: No se pudo crear mensaje1.txt\n");
    }

    // Liberar memoria
    delete[] mensaje;
    delete[] desencriptado;
    delete[] descomprimido;

    return 0;
}
