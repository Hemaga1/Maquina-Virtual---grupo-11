
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "maquinaVirtual.h"
#include "Instrucciones.h"

typedef char VecString[4];

int main(int argce, char *argve[])
{
    tipoMV mv;
    int argc = 3;
    char *argv[] = {NULL, "Ej11.vmx", "-d"};
    // Verifico que se haya ingresado el nombre del archivo
    if (argc >= 2)
    {
        if (leerEncabezado(argv[1], &mv)) {
            if (argc == 3 && strcmp(argv[2], "-d") == 0)
                Disassembler(mv);
            InicializarRegistros(mv.registros);
            ejecutar_maquina(&mv);
        }
    }
    else
    {
        printf("No se ha ingresado el nombre del archivo\n");
        return 1;
    }

    return 0;
}


int leerEncabezado(const char *filename, tipoMV *programa)
{
    FILE *arch = fopen(filename, "rb");

    char tamanios[2];
    short high, low;
    short tamanioCodigo;

    if (!arch)
    {
        printf("No se pudo abrir el archivo");
        return 0;
    }

    char id[6];
    fread(id, sizeof(char), 5, arch);

    // Verificar que el identificador sea correcto
    if (strcmp(id, IDENTIFICADOR) != 0)
    {
        fprintf(stderr, "Archivo no valido: identificador incorrecto (%s)\n", programa->nombreVMX);
        fclose(arch);
        return 0;
    }
    else
    {
        int version;
        fread(&(programa->version), 1, 1, arch);
        if (programa->version == VERSION)
        {
            //fseek(arch, 0, SEEK_SET);
            fread(tamanios, sizeof(char), 2, arch);
            high = tamanios[0] & 0x0FF;
            low = tamanios[1] & 0x0FF;
            tamanioCodigo = ((high << 8) | low);

            if (tamanioCodigo > TAMANIO_MEMORIA)
            {
                fclose(arch);
                printf("Error: Tamanio de code segment excede la memoria de la maquina virtual\n");
                return 0;
            }

            fread(programa->memoria, 1, tamanioCodigo, arch);

            programa->TS[0][1] = programa->TS[1][0] = tamanioCodigo;
            programa->TS[0][0] = 0;
            programa->TS[1][1] = TAMANIO_MEMORIA;
        }

        fclose(arch);
        return 1;
    }
}


void Disassembler(tipoMV programa)
{
    uint32_t dir = 0;
    uint32_t cantbytes;
    uint32_t op1, op2;
    VecString Vec[32];

    strcpy(Vec[10], "EAX");
    strcpy(Vec[11], "EBX");
    strcpy(Vec[12], "ECX");
    strcpy(Vec[13], "EDX");
    strcpy(Vec[14], "EEX");
    strcpy(Vec[15], "EFX");
    strcpy(Vec[16], "AC");
    strcpy(Vec[17], "CC");
    strcpy(Vec[26], "CS");
    strcpy(Vec[27], "DS");

    while (dir < programa.TS[1][0])
    {
        printf("[%04X] ", dir);

        op1 = ((programa.memoria[dir] & 0b110000) >> 4) << 24;
        op2 = ((programa.memoria[dir] & 0b11000000) >> 6) << 24;

        cantbytes = (op1 >> 24) + (op2 >> 24) + 1;
        for (int i = 0; i < cantbytes; i++)
        {
            printf("%02X ", programa.memoria[dir + i] & 0xFF);
        }
        while (cantbytes < 9)
        {
            printf("   ");
            cantbytes++;
        }
        printf("| ");
        printf("%s ", Mnemonicos[programa.memoria[dir] & 0x1F]);
        dir++;

        switch (op2 >> 24)
        {
        case 0:
            break;
        case 1:
            op2 += programa.memoria[dir];
            dir++;
            break;
        case 2:
            op2 += ((programa.memoria[dir] & 0xFF) << 8) + (programa.memoria[dir + 1] & 0xFF);
            if (op2 & 0x8000)
                op2 = (op2 & 0xFF00FFFF) + 0xFF0000;
            dir += 2;
            break;
        case 3:
            op2 += (programa.memoria[dir] << 16) + ((programa.memoria[dir + 1] & 0xFF) << 8) + (programa.memoria[dir + 2] & 0xFF);
            dir += 3;
            break;
        }

        switch (op1 >> 24)
        {
        case 0:
            break;
        case 1:
            op1 += programa.memoria[dir];
            dir++;
            break;
        case 2:
            op1 += (programa.memoria[dir] << 8) + (programa.memoria[dir + 1] & 0xFF);
            dir += 2;
            break;
        case 3:
            op1 += (programa.memoria[dir] << 16) + ((programa.memoria[dir + 1] & 0xFF) << 8) + (programa.memoria[dir + 2] & 0xFF);
            dir += 3;
            break;
        }

        if (((op1 >> 24) != 0) && ((op2 >> 24) != 0))
        {
            PrintOperando(op1, Vec);
            printf(", ");
            PrintOperando(op2, Vec);
        }
        else
        {
            if ((op2 >> 24) != 0)
                PrintOperando(op2, Vec);
        }
        printf("\n");
    }
}


void ejecutar_maquina(tipoMV *mv)
{
    funcion operaciones[32];
    tipoMV aux = *mv;
    short offset;
    short dir = mv->TS[0][0];
    uint8_t instruccion;
    uint16_t mascaraOPC = 0x01F; // mascara para obtener el codigo de operacion
    uint16_t mascaraTOP = 0x03;  // mascara para obtener tipos de operando
    uint8_t TOP1, TOP2;
    inicioVectorOper(operaciones);
    while ((mv->registros[IP] < ((mv->TS[0][1] + mv->TS[0][0]))) && (mv->registros[IP] != -1))
    {
        //Lectura de la instruccion
        uint32_t posicion = mv->registros[IP];
        instruccion = mv->memoria[posicion];
        mv->registros[OPC] = instruccion & mascaraOPC;
        TOP2 = (instruccion >> 6) & mascaraTOP;
        TOP1 = (instruccion >> 4) & mascaraTOP;

        mv->registros[IP] += 1;

        if (TOP2 != 0)
            leerOperando(mv, TOP2, OP2);
        else
            mv->registros[OP2] = 0;

        if (TOP1 != 0)
            leerOperando(mv, TOP1, OP1);
        else
            mv->registros[OP1] = 0;

        //Ejecucion de la instruccion
        if (mv->registros[OPC] >= 0 && mv->registros[OPC] < NUM_INSTRUCCIONES && operaciones[mv->registros[OPC]] != NULL)
            operaciones[mv->registros[OPC]](mv, mv->registros[OP1], mv->registros[OP2]);
        else
            printf("No hay función asociada.\n");
    }
}

void PrintOperando(uint32_t op, VecString Vec[])
{
    uint32_t valor;
    switch (op >> 24)
    {
    case 0:
        break;
    case 1:
        printf("%s", Vec[op & 0x1F]);
        break;
    case 2:
        if (op & 8000)
        {
            valor = op & 0xFFFF;
            valor = CambiarSigno(valor);
            printf("-%d", valor & 0xFFFF);
        }
        else
            printf("%d", op & 0xFFFF);
        break;
    case 3:
        printf("[");
        printf("%s", Vec[(op & 0x1F0000) >> 16]);
        if ((op & 0xFFFF) != 0)
        {
            if (op & 0x8000)
            {
                valor = op & 0xFFFF;
                valor = CambiarSigno(valor);
                printf("-%d", valor & 0xFFFF);
            }
            else
            {
                printf("+");
                printf("%d", op & 0xFFFF);
            }
        }
        printf("]");
        break;
    }
}

void InicializarRegistros(uint32_t registros[])
{
    for (int i = 0; i < 32; i++)
        registros[i] = 0;
    registros[DS] = 0x010000;
    registros[IP] = registros[CS];
}

void ModificarIP(tipoMV *programa, char valor)
{
    programa->registros[IP] = valor;
}

void leerOperando(tipoMV *mv, int TOP, uint32_t Op)
{
    uint32_t dir = mv->registros[IP]; // Dirección actual de instrucción

    switch (TOP)
    {
    case TRegistro:
    {
        mv->registros[Op] = (TRegistro << 24);
        mv->registros[Op] |= mv->memoria[dir];     // Guarda número de registro
        mv->registros[IP]++;                       // Avanza IP
    }
    break;

    case TInmediato:
    {
        int16_t valor = (mv->memoria[dir] << 8) | mv->memoria[dir + 1]; // 16 bits con signo

        if (valor & 0x8000)
            valor |= 0xFFFF0000; // Propagación de signo a 32 bits

        mv->registros[Op] = (TInmediato << 24) | (valor & 0x00FFFFFF); // Marca tipo y guarda valor
        mv->registros[IP] += 2;
    }
    break;

    case TMemoria:
    {
        int32_t valor = (mv->memoria[dir] << 16) | (mv->memoria[dir + 1] << 8) | mv->memoria[dir + 2];
        mv->registros[Op] = (TMemoria << 24) | (valor & 0x00FFFFFF);
        mv->registros[IP] += 3;
    }
    break;

        printf( "Error: Tipo de operando inválido (%d).\n", TOP);
        exit(EXIT_FAILURE);
    }
}

uint8_t getTipoOperando(uint32_t op)
{
    return op >> 24;
}

uint32_t CambiarSigno(uint32_t valor)
{
    return (valor ^ 0xFFFFFFFF) + 1;
}

void ModificarCC(tipoMV *programa, uint32_t resultado)
{
    if (resultado < 0)
        programa->registros[CC] = programa->registros[CC] | 1 << 31;
    else
    {
        programa->registros[CC] = 0;
        if (resultado == 0)
            programa->registros[CC] = programa->registros[CC] | 1 << 30;
    }
}

void SetearAccesoMemoria(tipoMV *programa, uint32_t OP, uint8_t bytes, uint32_t direccion_fisica)
{
    programa->registros[LAR] = programa->registros[(OP & 0x1F0000) >> 16] + (OP & 0xFFFF);
    programa->registros[MAR] = (programa->registros[MAR] & 0xFFFF0000) + direccion_fisica;
    programa->registros[MAR] = (programa->registros[MAR] & 0xFFFF) + (bytes << 16);
}


uint32_t getDireccionFisica(tipoMV programa, uint32_t direccion_logica) {
    /*uint32_t segmento = direccion_logica >> 16;
    uint32_t offset   = direccion_logica & 0xFFFF;

    // Valida que el segmento existe
    if (segmento >= 8) {
        printf("Fallo de segmento: segmento %u inválido\n", segmento);
        exit(1);
    }

    uint32_t base  = programa.TS[segmento][0];
    uint32_t limite = programa.TS[segmento][1];

    // Valida que el offset esté dentro del límite
    if (offset >= limite) {
        printf("Fallo de segmento: offset 0x%X fuera del límite 0x%X del segmento %u\n",
               offset, limite, segmento);
        exit(1);
    }*/

    //return base + offset;

     int segment, baseSeg, offSeg;
    segment = (programa.registros[LAR] & 0xFFFF0000) >> 16;  // obtengo el segmento

    if (segment > 7) {  // si el segmento es mayor a 7, error
        printf("Error: Segmentation fault.\n");
        exit(1);
    }

    baseSeg = programa.TS[segment][0];
    offSeg = programa.registros[LAR] & 0x0000FFFF;
    printf("Base: 0x%X Offset: 0x%X\n", baseSeg, offSeg);
    return 0x00000000 | (baseSeg + offSeg);
}


uint32_t PropagarSigno(uint32_t valor, uint32_t cant)
{
    if ((valor & 0x80000000) == 0x80000000)
        for (int i = 0; i < cant; i++)
        {
            valor = valor >> 1;
            valor += (1 << 31);
        }
    else
        valor = valor >> cant;
    return valor;
}

uint32_t getValorCargar(tipoMV *programa, uint32_t OP, uint8_t tipo_op)
{
    uint32_t direccion_fisica;
    if (tipo_op == 3)
    {
        direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16] + (OP & 0xFFFF));
        SetearAccesoMemoria(programa, OP, 1, direccion_fisica);
        programa->registros[MBR] = programa->memoria[direccion_fisica];
        return programa->registros[MBR];
    }
    else if (tipo_op == 1)
        return programa->registros[(OP & 0x1F)];
    else
        return PropagarSigno(((OP & 0xFFFF) << 16), 16);
}

void MostrarBinario(char numero)
{
    uint8_t Vec[200];
    char N = -1;
    while (numero != 0)
    {
        N++;
        Vec[N] = numero & 1;
        numero = numero >> 1;
    }
    for (int i = N; i >= 0; i--)
        printf("%d", Vec[i]);
}
=======
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
