#include <iostream>
#include <cstdlib>   // Para malloc, free
#include <cstring>   // Para memcmp
#include <cstdio>    // Para funciones de I/O de C: fopen, fread, fseek, ftell

using namespace std;

// =========================================================
// I. FUNCIONES DE BITWISE Y CIFRADO
// =========================================================

// 1. Funciones de Comprensión

// 1.1. Rotar bits a la izquierda (Rotación <-- n)
unsigned char rotarIzquierda(unsigned char valor, int n) {
    n = n & 7; // Asegura n en [0, 7]
    return (valor << n) | (valor >> (8 - n));
}

// 1.2. Rotar bits a la derecha (Rotación --> n)
unsigned char rotarDerecha(unsigned char valor, int n) {
    n = n & 7; // Asegura n en [0, 7]
    return (valor >> n) | (valor << (8 - n));
}

// 1.3. Operación XOR
unsigned char aplicarXOR(unsigned char valor, unsigned char clave) {
    return valor ^ clave;
}

// 2. Funciones de descompresión
// 2.1. Función para desencriptar un arreglo: XOR (K) luego Rotación Derecha (n)
void desencriptar(unsigned char *entrada, unsigned char *salida, int longitud, int n, unsigned char clave) {
    for (int i = 0; i < longitud; i++) {
        unsigned char temp = aplicarXOR(entrada[i], clave);
        salida[i] = rotarDerecha(temp, n);
    }
}

// =========================================================
// II. FUNCIÓN DE UTILIDAD: LECTURA DE ARCHIVOS
// =========================================================

/*
 * 3. Carga el contenido binario de un archivo a un buffer asignado dinámicamente.
 */
int cargarArchivoBinario(const char *nombre_archivo, unsigned char **buffer) {
    FILE *archivo = fopen(nombre_archivo, "rb");
    if (archivo == NULL) {
        cerr << "Error: No se pudo abrir el archivo " << nombre_archivo << endl;
        *buffer = nullptr;
        return 0;
    }

    // 3.1. Obtener la longitud del archivo
    fseek(archivo, 0, SEEK_END);
    long longitud = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);

    if (longitud <= 0) {
        fclose(archivo);
        *buffer = nullptr;
        return 0;
    }

    // 3.2. Asignar memoria dinámica
    *buffer = (unsigned char*)malloc(longitud);
    if (*buffer == nullptr) {
        cerr << "Error: Falla al asignar memoria para " << nombre_archivo << endl;
        fclose(archivo);
        return 0;
    }

    // 3.3. Leer el contenido del archivo
    size_t leidos = fread(*buffer, 1, longitud, archivo);

    fclose(archivo);

    if (leidos != (size_t)longitud) {
        cerr << "Error: Falla en la lectura completa de " << nombre_archivo << endl;
        free(*buffer);
        *buffer = nullptr;
        return 0;
    }

    return (int)longitud;
}

// =========================================================
// III. FUNCIONES DE DESCOMPRESIÓN RLE
// =========================================================

/*
 * Descomprime un mensaje usando Run-Length Encoding (RLE).
 * Formato de la entrada: (Longitud de 2 bytes - Big-endian) + (Símbolo de 1 byte).
 * Se asume que la longitud original NO excede el rango de un 'int'.
 */
int descomprimirRLE(unsigned char *entrada, int longitud_E, unsigned char **salida) {
    if (longitud_E == 0) {
        *salida = nullptr;
        return 0;
    }

    // El mensaje RLE debe ser un múltiplo de 3 para ser procesado como (2 bytes L, 1 byte S)
    if (longitud_E % 3 != 0) {
        // En un caso real de ingeniería inversa, esto podría indicar que la longitud
        // del archivo encriptado es incorrecta o que no es RLE.
        *salida = nullptr;
        return 0;
    }

    int longitud_original_estimada = 0;
    
    // 1. Estimar la longitud total (se requiere para la asignación de memoria inicial)
    for (int i = 0; i < longitud_E; i += 3) {
        // Concatenar 2 bytes en Big-Endian (Byte Alto, Byte Bajo) para obtener unsigned short (longitud_run)
        unsigned short longitud_run = (entrada[i] << 8) | entrada[i+1];
        longitud_original_estimada += longitud_run;
    }

    // 2. Asignar memoria para el mensaje original
    *salida = (unsigned char*)malloc(longitud_original_estimada + 1); // +1 para posible '\0'
    if (*salida == nullptr) {
        return 0; // Error de asignación de memoria
    }

    int idx_salida = 0;

    // 3. Rellenar el mensaje original
    for (int i = 0; i < longitud_E; i += 3) {
        unsigned short longitud_run = (entrada[i] << 8) | entrada[i+1]; 
        unsigned char simbolo = entrada[i+2];                           
        
        // Repetir el símbolo 'longitud_run' veces
        for (int j = 0; j < longitud_run; j++) {
            if (idx_salida < longitud_original_estimada) {
                (*salida)[idx_salida++] = simbolo;
            } else {
                // Falla en el cálculo de longitud
                free(*salida);
                *salida = nullptr;
                return 0;
            }
        }
    }

    // Asegurar el terminador nulo (útil si la pista se lee como texto ASCII)
    (*salida)[longitud_original_estimada] = '\0';
    
    return longitud_original_estimada;
}

// =========================================================
// IV. FUNCIÓN DE INGENIERÍA INVERSA (Motor de Búsqueda)
// =========================================================

/*
 * Intenta encontrar los parámetros (n, K) probando el método RLE.
 * Retorna: 1 si es RLE, 0 si no se encuentra.
 */
int identificarParametros(unsigned char *encriptado, int longitud_E, 
                          unsigned char *fragmento_original, int longitud_F, 
                          int *n_ptr, unsigned char *k_ptr) {
    
    // Iteración a través de todos los posibles valores de n y K
    for (int n_trial = 1; n_trial < 8; n_trial++) { // n: 1 a 7
        for (int k_trial = 0; k_trial <= 255; k_trial++) { // K: 0x00 a 0xFF
            
            // 1. Desencriptar el mensaje completo para el intento (n_trial, k_trial)
            unsigned char *temp_desencriptado = (unsigned char*) malloc(longitud_E);
            if (!temp_desencriptado) return 0; 

            desencriptar(encriptado, temp_desencriptado, longitud_E, n_trial, (unsigned char)k_trial);

            // 2. Intentar Descompresión RLE
            unsigned char *rle_resultado = nullptr;
            int rle_len = descomprimirRLE(temp_desencriptado, longitud_E, &rle_resultado);
            
            // Si la descompresión RLE fue exitosa y tiene al menos la longitud de la pista
            if (rle_resultado != nullptr && rle_len >= longitud_F) {
                // Compara los primeros 'longitud_F' bytes del resultado RLE con la pista
                if (memcmp(rle_resultado, fragmento_original, longitud_F) == 0) {
                    // ¡Éxito! Encontramos los parámetros RLE
                    *n_ptr = n_trial;
                    *k_ptr = (unsigned char)k_trial;
                    
                    // Limpieza de memoria
                    free(temp_desencriptado);
                    free(rle_resultado);
                    return 1; // RLE
                }
            }
            
            // Limpieza de memoria para el intento
            if(rle_resultado) free(rle_resultado);
            free(temp_desencriptado);
        }
    }

    return 0; // Parámetros RLE no encontrados
}

// =========================================================
// V. MAIN ORQUESTADOR
// =========================================================

int main() {
    // Rutas de los archivos en la carpeta 'datasetDesarrollo'
    const char *ruta_encriptado = "datasetDesarrollo/Encriptado1.txt";
    const char *ruta_pista = "datasetDesarrollo/pista1.txt";
    
    // Punteros para almacenar el contenido de los archivos
    unsigned char *encriptado = nullptr;
    unsigned char *fragmento_original = nullptr;
    
    // Longitudes
    int longitud_encriptado = 0;
    int longitud_fragmento = 0;

    // --- 1. Cargar Archivos ---
    cout << "Cargando archivos desde datasetDesarrollo/..." << endl;

    longitud_encriptado = cargarArchivoBinario(ruta_encriptado, &encriptado);
    if (longitud_encriptado == 0) {
        return 1; 
    }

    longitud_fragmento = cargarArchivoBinario(ruta_pista, &fragmento_original);
    if (longitud_fragmento == 0) {
        cout << "Error: No se pudo cargar la pista. Asegurese que pista1.txt contiene texto." << endl;
        free(encriptado); 
        return 1; 
    }
    
    cout << "  Encriptado1.txt cargado. Longitud: " << longitud_encriptado << " bytes." << endl;
    cout << "  Pista1.txt cargada. Longitud: " << longitud_fragmento << " bytes." << endl;
    cout << "  Pista conocida: '" << fragmento_original << "'" << endl;
    cout << "-----------------------------------------------" << endl;


    // --- 2. Variables para Ingeniería Inversa ---
    int n_final = 0;
    unsigned char k_final = 0;
    int metodo_final = 0;

    // --- 3. Ejecutar Ingeniería Inversa ---
    cout << "Iniciando Ingenieria Inversa (Prueba RLE para todos los n y K)..." << endl;
    
    metodo_final = identificarParametros(encriptado, longitud_encriptado, 
                                        fragmento_original, longitud_fragmento, 
                                        &n_final, &k_final);

    cout << "-----------------------------------------------" << endl;

    // --- 4. Procesamiento Final (Desencriptación y Descompresión) ---
    if (metodo_final == 1) { // RLE
        cout << "PARAMETROS IDENTIFICADOS (Metodo RLE)" << endl;
        cout << "  Parametro de rotacion n: " << n_final << endl;
        cout << "  Clave XOR K: 0x" << hex << (int)k_final << dec << endl;

        // Desencriptar el mensaje completo
        unsigned char *desencriptado = (unsigned char*) malloc(longitud_encriptado);
        if (!desencriptado) { free(encriptado); free(fragmento_original); return 1; }

        desencriptar(encriptado, desencriptado, longitud_encriptado, n_final, k_final);
        
        // Descomprimir RLE
        unsigned char *mensaje_original = nullptr;
        int longitud_original = descomprimirRLE(desencriptado, longitud_encriptado, &mensaje_original);
        
        cout << "-----------------------------------------------" << endl;

        if (mensaje_original != nullptr && longitud_original > 0) {
            cout << "Mensaje original reconstruido (" << longitud_original << " bytes):" << endl;
            // Imprimir la cadena completa
            for (int i = 0; i < longitud_original; i++) {
                cout << mensaje_original[i];
            }
            cout << endl;
            free(mensaje_original);
        } else {
            cout << "Error en la descompresion RLE final." << endl;
        }

        free(desencriptado);

    } else {
        cout << "No se encontraron parametros (n, K) que funcionen con el metodo RLE." << endl;
        cout << "El siguiente paso es implementar el metodo LZ78 y actualizar la funcion identificacionParametros." << endl;
    }

    // --- 5. Liberación de Memoria de Entrada ---
    if (encriptado) free(encriptado);
    if (fragmento_original) free(fragmento_original);
    
    return 0;
}