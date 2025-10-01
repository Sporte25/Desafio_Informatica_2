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

    // Liberar memoria
    delete[] mensaje;
    delete[] fragmento;

    return 0;
}
