#include <stdio.h>
#include <stdlib.h>

// Tamaños máximos
#define MAX_SIZE 10000
#define PISTA_MAX 100

// Prototipos
unsigned char rotate_right(unsigned char byte, int n);
void xor_and_rotate(unsigned char* input, unsigned char* output, int length, unsigned char key, int n);
int contains_subsequence(unsigned char* data, int data_len, unsigned char* sub, int sub_len);

// Lectura de archivo
int load_file(const char* filename, unsigned char* buffer) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("No se pudo abrir el archivo: %s\n", filename);
        return -1;
    }
    int i = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF && i < MAX_SIZE) {
        buffer[i++] = (unsigned char)ch;
    }
    fclose(file);
    return i;
}

// Rotación de bits a la derecha
unsigned char rotate_right(unsigned char byte, int n) {
    return (byte >> n) | (byte << (8 - n));
}

// Aplicar XOR y rotación inversa a cada byte
void xor_and_rotate(unsigned char* input, unsigned char* output, int length, unsigned char key, int n) {
    for (int i = 0; i < length; i++) {
        unsigned char x = input[i] ^ key;
        output[i] = rotate_right(x, n);
    }
}

// Buscar si contiene pista
int contains_subsequence(unsigned char* data, int data_len, unsigned char* sub, int sub_len) {
    for (int i = 0; i <= data_len - sub_len; i++) {
        int found = 1;
        for (int j = 0; j < sub_len; j++) {
            if (data[i + j] != sub[j]) {
                found = 0;
                break;
            }
        }
        if (found) return 1;
    }
    return 0;
}

int main() {
    unsigned char encrypted[MAX_SIZE];
    unsigned char pista[PISTA_MAX];
    unsigned char desencriptado[MAX_SIZE];

    int len_encrypted = load_file("Encriptado1.txt", encrypted);
    int len_pista = load_file("pista1.txt", pista);

    if (len_encrypted <= 0 || len_pista <= 0) {
        printf("Error leyendo archivos.\n");
        return 1;
    }

    // Probar combinaciones
    for (int n = 1; n < 8; n++) {
        for (int k = 0; k < 256; k++) {
            xor_and_rotate(encrypted, desencriptado, len_encrypted, (unsigned char)k, n);
            if (contains_subsequence(desencriptado, len_encrypted, pista, len_pista)) {
                printf(">>> POSIBLE COINCIDENCIA:\n");
                printf("Rotación: %d bits, Clave XOR: 0x%02X\n", n, k);
                printf("Primeros 100 bytes desencriptados:\n");
                for (int i = 0; i < 100 && i < len_encrypted; i++) {
                    printf("%c", desencriptado[i]);
                }
                printf("\n\n");
            }
        }
    }

    return 0;
}
