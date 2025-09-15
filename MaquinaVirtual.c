#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "maquinaVirtual.h"
#include "Instrucciones.h"


int leerEncabezado(const char *filename, tipoMV *programa) {
    FILE *arch = fopen(filename, "rb");

    char header[19];
    short high, low;
    short tamanio;


    if (!arch) {
        perror("No se pudo abrir el archivo");
        return 0;
    }
    char id[6];
    fread(id, sizeof(char), 5, arch);

    // Verificar que el identificador sea correcto
    if (strcmp(id, IDENTIFICADOR) != 0) {
        fprintf(stderr, "Archivo no v�lido: identificador incorrecto (%s)\n", programa->nombreVMX);
        fclose(arch);
        return 0;
    }
    else {
        int version;
        fread(&(programa->version), 1, 1, arch);
        if (programa->version == VERSION) {
            fseek(arch, 0, SEEK_SET);
            fread(header, sizeof(char), 8, arch);
            header[8] = '\0';

            high = header[6] & 0x0FF;
            low = header[7] & 0x0FF;
            tamanio = ((high << 8) | low);

            if (tamanio > TAMANIO_MEMORIA) {
                fclose(arch);
                printf("Error: Tamanio de code segment excede la memoria de la m�quina virtual\n");
                return 0;
            }
            for (int i=0; i<16384; i++)
                programa->memoria[i]=0;
            fread(programa->memoria, 1, tamanio, arch);
            programa->TS[0][1] = programa->TS[1][0] = tamanio;
            programa->TS[0][0] = 0;
            programa->TS[1][1] = TAMANIO_MEMORIA;
            // inicializar_maquina(maquina, tamanio);
        }
        fclose(arch);
        return 1;
    }

}

void InicializarRegistros(uint32_t registros[])
{
    for (int i = 0; i < 32; i++)
        registros[i] = 0;
    registros[DS] = 0x010000;
    registros[IP] = registros[CS];
}

void ModificarIP(tipoMV *programa, char valor){
    programa->registros[IP] = valor;
}

void leeroperando(tipoMV *mv, int TOP, uint32_t Op) {
    uint32_t dir = mv->registros[IP];  // Dirección actual de instrucción


    switch (TOP) {
        case 0b01: {  // Registro
            mv->registros[Op] = (TIPO_REGISTRO << 24);       // Marca tipo
            mv->registros[Op] |= mv->memoria[dir];           // Guarda número de registro
            mv->registros[IP]++;                      // Avanza IP
        }
            break;


        case 0b10:{  // Inmediato (2 bytes, con signo)
            int16_t valor = (mv->memoria[dir] << 8) | mv->memoria[dir + 1];  // 16 bits con signo

            if (valor & 0x8000)
                valor |= 0xFFFF0000; // Propagación de signo a 32 bits

            mv->registros[Op] = (TIPO_INMEDIATO << 24) | (valor & 0x00FFFFFF); // Marca tipo y guarda valor
            mv->registros[IP] += 2;
        }
            break;


        case 0b11: {  // Dirección de memoria (3 bytes, posiblemente con signo)
            int32_t valor = (mv->memoria[dir] << 16) | (mv->memoria[dir + 1] << 8) | mv->memoria[dir + 2];
            mv->registros[Op] = (TIPO_MEMORIA << 24) | (valor & 0x00FFFFFF);
            mv->registros[IP] += 3;
        }
            break;


        default:
            fprintf(stderr, "Error: Tipo de operando inválido (%d).\n", TOP);
            exit(EXIT_FAILURE);
    }
}

void ejecutar_maquina(tipoMV *mv)
{
    funcion operaciones[32];
    // cambiar tabla a matriz
    short offset;
    short dir = mv->TS[0][0];
    uint8_t instruccion;
    uint16_t mascaraOPC = 0x01F;   // mascara para obtener el codigo de operacion
    uint16_t mascaraTOP = 0x03;  // mascara para obtener tipos de operando
    uint8_t TOP1, TOP2;
    inicioVectorOper(operaciones);
    while ((mv->registros[IP] < ((mv->TS[0][1] + mv->TS[0][0]))) && (mv->registros[IP] != -1))
    {
        // LECTURA INSTRUCCION
        uint32_t posicion = mv->registros[IP];
        instruccion = mv->memoria[posicion];
        mv->registros[OPC] = instruccion & mascaraOPC;
        TOP2 = (instruccion >> 6) & 0x03;
        TOP1 = (instruccion >> 4) & 0x03;

        mv->registros[IP] = mv->registros[IP] + 1;

        if (TOP2 != 0)
            leeroperando(mv, TOP2, OP2);
        else
            mv->registros[OP2] = 0;

        if (TOP1 != 0)
            leeroperando(mv, TOP1, OP1);
        else
            mv->registros[OP1] = 0;

        // EJECUCION INSTRUCCION
        // printf("%d", mv->registros[OPC]);
        operaciones[mv->registros[OPC]](mv, mv->registros[OP1], mv->registros[OP2]);
    }
}

uint8_t getTipoOperando(uint32_t op){
    return op >> 24;
}

uint32_t getDireccionFisica(tipoMV programa, uint32_t direccion_logica){
    return programa.TS[direccion_logica >> 16][0] + (direccion_logica & 0xFFFF);
}

uint32_t CambiarSigno(uint32_t valor){
    return (valor ^ 0xFFFFFFFF) + 1;
}

void ModificarCC(tipoMV *programa, uint32_t resultado){
    if (resultado < 0)
        programa->registros[CC] = programa->registros[CC] | 1 << 31;
    else{
        programa->registros[CC] = 0;
        if (resultado == 0)
            programa->registros[CC] = programa->registros[CC] | 1 << 30;
    }
}

void SetearAccesoMemoria(tipoMV *programa, uint32_t OP, uint8_t bytes, uint32_t direccion_fisica){
    programa->registros[LAR] = programa->registros[(OP & 0x1F0000) >> 16] + (OP & 0xFFFF);
    programa->registros[MAR] = (programa->registros[MAR] & 0xFFFF0000) + direccion_fisica;
    programa->registros[MAR] = (programa->registros[MAR] & 0xFFFF) + (bytes << 16);
}



uint32_t PropagarSigno(uint32_t valor, uint32_t cant){
    if ((valor & 0x80000000) == 0x80000000)
        for (int i=0;i<cant;i++){
            valor = valor >> 1;
            valor += (1 << 31);
        }
    else valor = valor >> cant;
    return valor;
}

uint32_t getValorCargar(tipoMV *programa, uint32_t OP, uint8_t tipo_op){
    uint32_t direccion_fisica;
    if (tipo_op == 3){
        direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16]+ (OP & 0xFFFF));
        SetearAccesoMemoria(programa, OP, 1, direccion_fisica);
        programa->registros[MBR] = programa->memoria[direccion_fisica];
        return programa->registros[MBR];
    }
    else
        if (tipo_op == 1)
            return programa->registros[(OP & 0x1F)];
        else return PropagarSigno(((OP & 0xFFFF) << 16),16);
}

void MostrarBinario(char numero){
    uint8_t Vec[200];
    char N = -1;
    while (numero != 0){
        N++;
        Vec[N] = numero & 1;
        numero = numero >> 1;
    }
    for (int i=N;i>=0;i--)
        printf("%d",Vec[i]);
}

int main() {
    tipoMV mv;
    tipoInstruccion* instrucciones;
    int instruccion_size = 0;
    char *argv[] = {"sample.vmx", NULL};// Simulando el argumento de linea de comandos
    int argc = 2;
    // Verifico que se haya ingresado el nombre del archivo
    if (argc > 1 && leerEncabezado(argv[0], &mv) ) {
        InicializarRegistros(mv.registros);
        ejecutar_maquina(&mv);
    }
    else {
        printf("No se ha ingresado el nombre del archivo\n");
        return 1;
    }

    return 0;
}
