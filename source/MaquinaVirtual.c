#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "maquinaVirtual.h"
#include "Instrucciones.h"

int main(int argc, char *argv[])
{
    tipoMV mv;
    Tparametros parametros;

    leerParametros(argc, argv, &parametros);

    mv.tamanioMemoria = parametros.tamanioMemoria;
    crearParamSegment(&mv, &parametros);

    if (parametros.disassembler)
        Disassembler(mv);
    
    if (parametros.vmxfile)
    { // Si hay .vmx lo leo
        leerVMX(&mv, parametros.vmxfile);
        if (parametros.vmifile) // Si tambien habia .vmi genero la imagen
            escribirVMI(&mv, parametros.vmifile);
    }
    else if (parametros.vmifile) // Si no habia .vmx pero si .vmi lo leo para ejecutar la imagen
        leerVMI(&mv, parametros.vmifile);
    else
    {
        printf("No se ingreso el archivo .vmx o .vmi\n");
        return 1;
    }
    ejecutar_maquina(&mv);
    return 0;
}

void leerParametros(int argc, char *argv[], Tparametros *parametros) {
    
    parametros->tamanioMemoria = 16 * 1024;  
    parametros->disassembler = 0;
    parametros->argc = 0;
    parametros->vmxfile = NULL;
    parametros->vmifile = NULL;

    for (int i = 1; i < argc; i++) {
        if (strstr(argv[i], ".vmx")) {
            parametros->vmxfile = argv[i];
        } else if (strstr(argv[i], ".vmi")) {
            parametros->vmifile = argv[i];
        } else if (strncmp(argv[i], "m=", 2) == 0) {
            parametros->tamanioMemoria = atoi(argv[i] + 2) * 1024;  
        } else if (strcmp(argv[i], "-d") == 0) {
            parametros->disassembler = 1;
        } else if (strcmp(argv[i], "-p") == 0) {
            for (int j = i + 1; j < argc; j++) {
                parametros->constantes[parametros->argc++] = argv[j];
            }
            break;
        } else {
            printf("Argumento desconocido: %s\n", argv[i]);
        }
    }
}


void crearParamSegment(tipoMV *mv, Tparametros *parametros) {
    unsigned int base = 0;  
    unsigned int offset = 0;

    if (parametros->argc == 0) {
        mv->registros[PS] = -1;
        return;
    }

    // Copiar los strings uno detrás del otro
    for (int i = 0; i < parametros->argc; i++) {
        int len = strlen(parametros->constantes[i]) + 1;  
        memcpy(&mv->memoria[offset], parametros->constantes[i], len);
        offset += len;
    }
    // Configurar el segmento en la tabla
    mv->TS[0][0] = base;
    mv->TS[0][1] = offset;

    // Registrar el segmento PS
    mv->registros[PS] = base;
}

int leerVMX(const char *filename, tipoMV *mv)
{
    FILE *arch = fopen(filename, "rb");

    char tamanios[2];
    short high, low;
    short tamanioCS;

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
        fprintf(stderr, "Archivo no valido: identificador incorrecto (%s)\n", mv->nombreVMX);
        fclose(arch);
        return 0;
    }
    else
    {
        fread(&(mv->version), 1, 1, arch);
        if (mv->version == 1)
        {
            fread(tamanios, sizeof(char), 2, arch);
            high = tamanios[0] & 0x0FF;
            low = tamanios[1] & 0x0FF;
            tamanioCS = ((high << 8) | low);

            mv->registros[CS] = 0x0000;
            mv->registros[PS] = -1;
            mv->registros[KS] = -1;
            mv->registros[ES] = -1;
            mv->registros[SS] = -1;

            if (tamanioCS > mv->tamanioMemoria)
            {
                fclose(arch);
                printf("Error: Tamanio de code segment excede la memoria de la maquina virtual\n");
                return 0;
            }

            fread(mv->memoria, 1, tamanioCS, arch);

            mv->TS[0][1] = mv->TS[1][0] = tamanioCS;
            mv->TS[0][0] = 0;
            mv->TS[1][1] = mv->tamanioMemoria;
        }
        else if (mv->version == 2){

            fread(tamanios, sizeof(char), 2, arch);
            high = tamanios[0] & 0x0FF;
            low = tamanios[1] & 0x0FF;
            tamanioCS = ((high << 8) | low);

            unsigned int tamaniosSeg[5]; // CS, DS, ES, SS, KS

            for (int i = 0; i < 5; i++)
            {
                unsigned char buffer[2];
                if (fread(buffer, sizeof(char), 2, arch) != 2)
                {
                    fprintf(stderr, "ERROR: no se pudo leer tamaño de segmento\n");
                    fclose(arch);
                    exit(1);
                }
                tamaniosSeg[i] = (buffer[0] << 8) | buffer[1];
            }

            iniciarTablaSegmentos(mv, tamaniosSeg, 5);

            unsigned char entryPoint[2];
            if (fread(entryPoint, sizeof(char), 2, arch) != 2) {
                printf("ERROR: no se pudo leer el entry point\n");
                exit(1);
            }
            unsigned int tamanioEntryPoint = (entryPoint[0] << 8) | entryPoint[1];

            

            // inicializo registros 
        }
        else {
            printf("ERROR: versión de archivo incorrecta (%d)\n", mv->version);
            exit(1);
        }

        fclose(arch);
        return 1;
    }
}

void iniciarTablaSegmentos(tipoMV *mv, unsigned int sizes[], unsigned short int cantSegments) {
    // sizes = {CS, DS, ES, SS, KS}
    unsigned int base = 0;
    int indiceTS = 0;

    if(mv->registros[PS] != -1) {
        base += mv->TS[0][1];
        indiceTS += 1;
    }

    //  Primero el segmento de constantes si existe
    if(sizes[cantSegments - 1] > 0) {
        mv->TS[indiceTS][0] = base; 
        mv->TS[indiceTS][1] = sizes[cantSegments - 1];             
        base += mv->TS[indiceTS][1];
        indiceTS++;
    } 

    //  Luego los demás (CS, DS, ES, SS)
    for (int i = 0; i < cantSegments - 1; i++) {
        if (sizes[i] > 0) {
            mv->TS[indiceTS][0] = base; // tamaño
            mv->TS[indiceTS][1] = sizes[i];     // base acumulada
            base += sizes[i];
            indiceTS++;
        }
    }
}


void Disassembler(tipoMV programa)
{
    uint32_t dir = 0;
    uint32_t cantbytes;
    uint32_t op1, op2;

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
        uint8_t instruccion = programa.memoria[dir] & 0x1F;
        if (instruccion >= 0 && instruccion < NUM_INSTRUCCIONES) {
            printf("%s ", Mnemonicos[programa.memoria[dir] & 0x1F]);
        }
        else
             printf("%s ","Codigo de instruccion Invalido");
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
            PrintOperando(op1);
            printf(", ");
            PrintOperando(op2);
        }
        else
        {
            if ((op2 >> 24) != 0)
                PrintOperando(op2);
        }
        printf("\n");
    }
}

void PrintOperando(uint32_t op){
    char registro[4];
    uint32_t valor;
    switch (op >> 24) {
                case 0:
                    break;
                case 1: strcpy(registro,NombreRegistro[op & 0x1F]);
                        switch (op & 0xC0){
                            case 0x00: printf("%s",registro);
                                break;
                            case 0x01: printf("%cL",registro[1]);
                                break;
                            case 0x10: printf("%cH",registro[1]);
                                break;
                            case 0x11: printf("%cX",registro[1]);
                                break;
                        }
                    break;
                case 2: if (op & 0x8000){
                            valor = op & 0xFFFF;
                            valor = CambiarSigno(valor);
                            printf("-%d",valor & 0xFFFF);
                        }
                        else printf("%d",op & 0xFFFF);
                    break;
                case 3: switch (op & 0xC00000){
                            case 0b00: printf("l");
                                break;
                            case 0b10: printf("w");
                                break;
                            case 0b11: printf("b");
                                break;
                        }

                        printf("[");
                        printf("%s", NombreRegistro[(op & 0x1F0000) >> 16]);
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
            valor |= 0xFFFF0000; // Propagación de signo a 16 bits

        mv->registros[Op] = (TInmediato << 24) | (valor & 0x00FFFFFF); // Marca tipo y guarda valor
        mv->registros[IP] += 2;
    }
    break;

    case TMemoria:
    {
        uint32_t valor = (mv->memoria[dir] << 16) | (mv->memoria[dir + 1] << 8) | mv->memoria[dir + 2];
        mv->registros[Op] = (TMemoria << 24) | (valor & 0x00FFFFFF);
        mv->registros[IP] += 3;
    }
    break;

        printf( "Error: Tipo de operando inválido (%d).\n", TOP);
        exit(1);
    }
}

void ejecutar_maquina(tipoMV *mv)
{
    funcion operaciones[32];
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
        TOP2 = (instruccion >> 6) & mascaraTOP;
        TOP1 = (instruccion >> 4) & mascaraTOP;

        mv->registros[IP] = mv->registros[IP] + 1;


        if (TOP2 != 0)
            leerOperando(mv, TOP2, OP2);
        else
            mv->registros[OP2] = 0;

        if (TOP1 != 0)
            leerOperando(mv, TOP1, OP1);
        else
            mv->registros[OP1] = 0;
        // EJECUCION INSTRUCCION

        uint32_t aux =mv->registros[OPC];

        if (mv->registros[OPC] >= 0 && mv->registros[OPC] < NUM_INSTRUCCIONES && operaciones[mv->registros[OPC]] != NULL)
            operaciones[mv->registros[OPC]](mv, mv->registros[OP1], mv->registros[OP2]);
        else {
            printf("ERROR: Instruccion Invalida.\n");
            return exit(1);
        }

    }
}

uint8_t getTipoOperando(uint32_t op){
    return op >> 24;
}

uint32_t getDireccionFisica(tipoMV programa, uint32_t direccion_logica){
    uint32_t segmento = direccion_logica >> 16;
    uint32_t offset   = direccion_logica & 0xFFFF;

    // Valida que el segmento exista
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
    }

    return base + offset;
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
    programa->registros[MAR] = direccion_fisica;
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

uint32_t getValorCargar(tipoMV *programa, uint32_t OP){
    uint32_t direccion_fisica;
    uint8_t tipo_op = getTipoOperando(OP);


    if (tipo_op == 3){
        uint8_t cant_bytes = 4;
        if (programa->version == 2)
           cant_bytes -= (OP & 0xC00000);
        if (programa->registros[(OP & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16]+ (OP & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (OP & 0xFFFF));
        SetearAccesoMemoria(programa, OP, cant_bytes, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<cant_bytes; i++){
            programa->registros[MBR] |= programa->memoria[direccion_fisica + cant_bytes - 1 - i] << (i*8);
        }
        return programa->registros[MBR];
    }
    else
        if (tipo_op == 1){
            if (programa->version == 2){
                uint32_t res;
                switch (OP & 0xC0){
                    case 0b00: res = programa->registros[OP & 0x1F];
                        break;
                    case 0b01: res = programa->registros[OP & 0x1F] & 0xFF;
                        break;
                    case 0b10: res = programa->registros[OP & 0x1F] & 0xFF00;
                        break;
                    case 0b11: res = programa->registros[OP & 0x1F] & 0xFFFF;
                        break;
                }
                return res;
            }
            else return programa->registros[(OP & 0x1F)];
        }
        else return PropagarSigno(((OP & 0xFFFF) << 16),16);
}

void setOperando(tipoMV *programa, uint32_t OP, uint32_t valor_cargar){
    uint32_t direccion_fisica;
    uint8_t tipo_op = getTipoOperando(OP);

    if (tipo_op == 3){
        uint8_t cant_bytes = 4;
        if (programa->version == 2)
           cant_bytes -= (OP & 0xC00000);
        if (programa->registros[(OP & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16]+ (OP & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (OP & 0xFFFF));
        SetearAccesoMemoria(programa, OP, cant_bytes, direccion_fisica);
        programa->registros[MBR] = valor_cargar;

        for (int i=0; i<cant_bytes ; i++){
            programa->memoria[direccion_fisica + cant_bytes - 1 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
    }
    else {
        if (programa->version == 2){
            switch (OP & 0xC0){
                case 0b00: programa->registros[OP & 0x1F] = valor_cargar;
                    break;
                case 0b01: programa->registros[OP & 0x1F] = (programa->registros[OP & 0x1F] & 0xFFFFFF00) + (valor_cargar & 0xFF);
                    break;
                case 0b10: programa->registros[OP & 0x1F] = (programa->registros[OP & 0x1F] & 0xFFFF00FF) + (valor_cargar & 0xFF00);
                    break;
                case 0b11: programa->registros[OP & 0x1F] = (programa->registros[OP & 0x1F] & 0xFFFF0000) + (valor_cargar & 0xFFFF);
                    break;
            }
        }
        else programa->registros[OP & 0x1F] = valor_cargar;
    }
}

uint32_t StringABinario(char cadena[33]){
    uint32_t suma = 0;
    if (cadena[31] == '1')
        for (int i=32;i>-1;i--){
            if (cadena[i] == '1')
                suma += (1 << (31-i));
        }
    else
        for (int i=32;i>-1;i--){
            if (cadena[i] == '1')
                suma += (0x80000000 >> (31-i));
        }
    return suma;
}

void MostrarBinario(uint32_t numero){
    if (numero != 0){
        MostrarBinario(numero >> 1);
        if(numero & 1)
            printf("1");
        else
            printf("0");
    }
}

void pushearValor(tipoMV *programa, uint32_t valor){
    uint32_t direccion_fisica;

    programa->registros[SP] -= 4;
    if (programa->registros[SP] < programa->registros[SS]){
        printf("STACK OVERFLOW\n");
        exit(1);
    }
    else {
        direccion_fisica = getDireccionFisica(*programa,  programa->registros[SP]);

        programa->registros[LAR] = programa->registros[SP];
        programa->registros[MAR] = direccion_fisica;
        programa->registros[MAR] = (programa->registros[MAR] & 0xFFFF) + (4 << 16);
        programa->registros[MBR] = valor;

        for (int i=0; i<4 ; i++){
            programa->memoria[direccion_fisica + 3 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
    }
}

const char *NombreRegistro[28] = {
    [0] = "LAR",
    [1] = "MAR",
    [2] = "MBR",

    [3] = "IP",
    [4] = "OPC",
    [5] = "OP1",
    [6] = "OP2",

    [10] = "EAX",
    [11] = "EBX",
    [12] = "ECX",
    [13] = "EDX",
    [14] = "EEX",
    [15] = "EFX",

    [16] = "AC",
    [17] = "CC",

    [26] = "CS",
    [27] = "DS"
};
