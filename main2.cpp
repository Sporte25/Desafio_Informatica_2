#include <cstdio>
#include <cstdlib>
#include <cstring>

// Cargar archivo en memoria
unsigned char* cargarArchivo(const char* ruta, int* longitud) {
    FILE* archivo = fopen(ruta, "rb");
    if (!archivo) {
        printf("No se pudo abrir el archivo: %s\n", ruta);
        return nullptr;
    }

    fseek(archivo, 0, SEEK_END);
    int tam = ftell(archivo);
    rewind(archivo);

    unsigned char* buffer = new unsigned char[tam];
    fread(buffer, sizeof(unsigned char), tam, archivo);
    fclose(archivo);

    *longitud = tam;
    return buffer;
}

// Rotación a la izquierda
unsigned char rotarIzquierda(unsigned char byte, int n) {
    return (byte << n) | (byte >> (8 - n));
}

// XOR con clave
void desencriptar(unsigned char* datos, int longitud, int n, unsigned char k) {
    for (int i = 0; i < longitud; ++i) {
        datos[i] = rotarIzquierda(datos[i], n);
        datos[i] ^= k;
    }
}

// Comparar pista dentro del buffer
bool contienePista(unsigned char* datos, int longitud, const char* pista, int pistaLen) {
    for (int i = 0; i <= longitud - pistaLen; ++i) {
        bool coincide = true;
        for (int j = 0; j < pistaLen; ++j) {
            if (datos[i + j] != pista[j]) {
                coincide = false;
                break;
            }
        }
        if (coincide) return true;
    }
    return false;
}

// Prueba de desencriptación
void probarDesencriptacion(const char* archivoEncriptado, const char* archivoPista) {
    int lenEncriptado = 0;
    unsigned char* datos = cargarArchivo(archivoEncriptado, &lenEncriptado);
    if (!datos) return;

    int lenPista = 0;
    unsigned char* pista = cargarArchivo(archivoPista, &lenPista);
    if (!pista) {
        delete[] datos;
        return;
    }

    for (int n = 1; n < 8; ++n) {
        for (int k = 0; k < 256; ++k) {
            unsigned char* copia = new unsigned char[lenEncriptado];
            memcpy(copia, datos, lenEncriptado);

            desencriptar(copia, lenEncriptado, n, (unsigned char)k);

            if (contienePista(copia, lenEncriptado, (char*)pista, lenPista)) {
                printf("✅ Posible combinación encontrada:\n");
                printf("n = %d, k = %d\n", n, k);
                printf("Fragmento desencriptado:\n");

                for (int i = 0; i < lenEncriptado; ++i) {
                    unsigned char c = copia[i];
                    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                        putchar(c);
                    }
                }
                printf("\n\n");
            }

            delete[] copia;
        }
    }

    delete[] datos;
    delete[] pista;
}

// Punto de entrada
int main() {
    const char* rutaEncriptado = "datasetDesarrollo/Encriptado1.txt";
    const char* rutaPista = "datasetDesarrollo/pista1.txt";

    probarDesencriptacion(rutaEncriptado, rutaPista);

    return 0;
}