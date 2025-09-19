#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "maquinaVirtual.h"
#include "Instrucciones.h"

typedef char VecString[4];

void PrintOperando(uint32_t op, VecString Vec[]){
    uint32_t valor;
    switch (op >> 24) {
                case 0:
                    break;
                case 1: printf("%s",Vec[op & 0x1F]);
                    break;
                case 2: if (op & 0x8000){
                            valor = op & 0xFFFF;
                            valor = CambiarSigno(valor);
                            printf("-%d",valor & 0xFFFF);
                        }
                        else printf("%d",op & 0xFFFF);
                    break;
                case 3: printf("[");
                        printf("%s", Vec[(op & 0x1F0000) >> 16]);
                        if ((op & 0xFFFF) != 0){
                            if (op & 0x8000){
                                valor = op & 0xFFFF;
                                valor = CambiarSigno(valor);
                                printf("-%d",valor & 0xFFFF);
                            }
                            else {
                                printf("+");
                                printf("%d",op & 0xFFFF);
                            }
                        }
                        printf("]");
                    break;
            }
}

void Disassembler(tipoMV programa){
    uint32_t dir = 0;
    uint32_t cantbytes;
    uint32_t op1, op2;
    VecString Vec[32];

    strcpy(Vec[10],"EAX");
    strcpy(Vec[11],"EBX");
    strcpy(Vec[12],"ECX");
    strcpy(Vec[13],"EDX");
    strcpy(Vec[14],"EEX");
    strcpy(Vec[15],"EFX");
    strcpy(Vec[16],"AC");
    strcpy(Vec[17],"CC");
    strcpy(Vec[26],"CS");
    strcpy(Vec[27],"DS");

    while (dir < programa.TS[1][0]){
            printf("[%04X] ",dir);

            op1 = ((programa.memoria[dir] & 0b110000) >> 4) << 24;
            op2 = ((programa.memoria[dir] & 0b11000000) >> 6) << 24;


            cantbytes = (op1 >> 24) + (op2 >> 24) + 1;
            for (int i=0;i<cantbytes;i++){
                printf("%02X ", programa.memoria[dir+i] & 0xFF);
            }
            while (cantbytes < 9){
                printf("   ");
                cantbytes++;
            }
            printf("| ");

            switch (programa.memoria[dir] & 0x1F){
                case 0b10000: printf("MOV ");
                    break;
                case 0b10001: printf("ADD ");
                    break;
                case 0b10010: printf("SUB ");
                    break;
                case 0b10011: printf("MUL ");
                    break;
                case 0b10100: printf("DIV ");
                    break;
                case 0b10101: printf("CMP ");
                    break;
                case 0b10110: printf("SHL ");
                    break;
                case 0b10111: printf("SHR ");
                    break;
                case 0b11000: printf("SAR ");
                    break;
                case 0b11001: printf("AND ");
                    break;
                case 0b11010: printf("OR ");
                    break;
                case 0b11011: printf("XOR ");
                    break;
                case 0b11100: printf("SWAP ");
                    break;
                case 0b11101: printf("LDL ");
                    break;
                case 0b11110: printf("LDH ");
                    break;
                case 0b11111: printf("RND ");
                    break;
                case 0b00000: printf("SYS ");
                    break;
                case 0b00001: printf("JMP ");
                    break;
                case 0b00010: printf("JZ ");
                    break;
                case 0b00011: printf("JP ");
                    break;
                case 0b00100: printf("JN ");
                    break;
                case 0b00101: printf("JNZ ");
                    break;
                case 0b00110: printf("JNP ");
                    break;
                case 0b00111: printf("JNN ");
                    break;
                case 0b01000: printf("NOT ");
                    break;
                case 0b01111: printf("STOP ");
                    break;
            }

            dir++;

            switch (op2 >> 24) {
                case 0:
                    break;
                case 1: op2 += programa.memoria[dir];
                        dir++;
                    break;
                case 2: op2 += ((programa.memoria[dir] & 0xFF) << 8) + (programa.memoria[dir+1] & 0xFF);
                        if (op2 &0x8000)
                            op2 = (op2 & 0xFF00FFFF) + 0xFF0000;
                        dir += 2;
                    break;
                case 3: op2 += (programa.memoria[dir] << 16) + ((programa.memoria[dir+1] & 0xFF) << 8) + (programa.memoria[dir+2] & 0xFF);
                        dir += 3;
                    break;

            }

            switch (op1 >> 24) {
                case 0:
                    break;
                case 1: op1 += programa.memoria[dir];
                        dir++;
                    break;
                case 2: op1 += (programa.memoria[dir] << 8) + (programa.memoria[dir+1] & 0xFF);
                        dir += 2;
                    break;
                case 3: op1 += (programa.memoria[dir] << 16) + ((programa.memoria[dir+1] & 0xFF) << 8) + (programa.memoria[dir+2] & 0xFF);
                        dir += 3;
                    break;

            }

            if (((op1 >> 24) != 0) && ((op2 >> 24) != 0)){
                PrintOperando(op1, Vec);
                printf(", ");
                PrintOperando(op2, Vec);
            }
            else {
                if ((op2 >> 24) != 0)
                    PrintOperando(op2, Vec);
            }
            printf("\n");
    }
    printf("\n");
}


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

void ModificarIP(tipoMV *programa, uint32_t valor){
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
            uint16_t valor = (mv->memoria[dir] << 8) | mv->memoria[dir + 1];  // 16 bits con signo

            if (valor & 0x8000)
                valor |= 0xFFFF0000; // Propagación de signo a 32 bits

            mv->registros[Op] = (TIPO_INMEDIATO << 24) | (valor & 0x00FFFFFF); // Marca tipo y guarda valor
            mv->registros[IP] += 2;
        }
            break;


        case 0b11: {  // Dirección de memoria (3 bytes, posiblemente con signo)
            uint32_t valor = (mv->memoria[dir] << 16) | (mv->memoria[dir + 1] << 8) | mv->memoria[dir + 2];
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
    programa->registros[CC] = 0;
    if (resultado & 0x80000000)
        programa->registros[CC] = programa->registros[CC] | (1 << 31);
    else{
        if (resultado == 0)
            programa->registros[CC] = programa->registros[CC] | (1 << 30);
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

uint32_t getValorCargar(tipoMV *programa, uint32_t OP, uint8_t tipo_op, uint8_t bytes){
    uint32_t direccion_fisica;
    if (tipo_op == 3){
        if (programa->registros[(OP & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16]+ (OP & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (OP & 0xFFFF));
        SetearAccesoMemoria(programa, OP, bytes, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<bytes; i++){
            programa->registros[MBR] += programa->memoria[direccion_fisica + bytes - i] << (i*8);
        }
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
        Disassembler(mv);
        InicializarRegistros(mv.registros);
        ejecutar_maquina(&mv);
    }
    else {
        printf("No se ha ingresado el nombre del archivo\n");
        return 1;
    }
    //system("pause");
    return 0;
}
