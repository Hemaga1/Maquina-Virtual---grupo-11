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
    if (parametros.vmxfile)
    {
        mv.nombreVMX = parametros.vmxfile;
        leerVMX(parametros.vmxfile, &mv, &parametros);
        if (parametros.vmifile)
            mv.nombreVMI = parametros.vmifile;
        else
            mv.nombreVMI = NULL;

    }
    else if (parametros.vmifile) {
        mv.nombreVMI = parametros.vmifile;
        leerVMI(&mv, parametros.vmifile);
    }
    else
    {
        printf("No se ingreso el archivo .vmx o .vmi\n");
        return 1;
    }
    if (parametros.disassembler)
        Disassembler(mv);
    ejecutar_maquina(&mv);
    return 0;
}

void leerParametros(int argc, char *argv[], Tparametros *parametros) {
    // se inicializan los valores por defecto
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
    uint32_t argv[50];

    if (parametros->argc == 0) {
        mv->registros[PS] = -1;
    }
    else {
        // Copiar los strings uno detrás del otro
        for (int i = 0; i < parametros->argc; i++) {
            int len = strlen(parametros->constantes[i]) + 1;
            memcpy(&mv->memoria[offset], parametros->constantes[i], len);
            argv[i] = offset;
            offset += len;
        }

        mv->argv = offset;
        mv->argc = parametros->argc;

        for (int i = 0; i < parametros->argc; i++) {
            for (int j=0; j<4; j++)
                mv->memoria[offset + 3 - j] = (argv[i] & (0xFF << (j*8))) >> (j*8);
            offset += 4;
        }

        // Configurar el segmento en la tabla
        mv->TS[0][0] = base;
        mv->TS[0][1] = offset;

        // Registrar el segmento PS
        mv->registros[PS] = base;
    }
}

int leerVMX(const char *filename, tipoMV *mv, Tparametros *parametros)
{
    printf("Leyendo archivo VMX: %s\n", filename);
    FILE *arch = fopen(filename, "rb");

    char tamanios[2];
    short high, low;
    short tamanioCS;

    if (!arch)
    {
        printf("No se pudo abrir el archivo");
        exit(1);
    }

    char id[6];
    fread(id, sizeof(char), 5, arch);
    id[5] = '\0';

    // Verificar que el identificador sea correcto
    if (strcmp(id, IDENTIFICADOR) != 0)
    {
        fprintf(stderr, "Archivo no valido: identificador incorrecto (%s)\n", mv->nombreVMX);
        fclose(arch);
        return 0;
    }
    else
    {
        printf("Leyendo archivo VMX: %s\n", filename);
        fread(&(mv->versionVMX), 1, 1, arch);
        if (mv->versionVMX == 1)
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

            /* reservar memoria para todo el espacio de la MV antes de leer el code segment */
            mv->memoria = (char *)malloc(mv->tamanioMemoria);
            if (mv->memoria == NULL) {
                fprintf(stderr, "ERROR: no se pudo asignar memoria\n");
                fclose(arch);
                return 0;
            }

            fread(mv->memoria, 1, tamanioCS, arch);

            mv->TS[0][1] = mv->TS[1][0] = tamanioCS;
            mv->TS[0][0] = 0;
            mv->TS[1][1] = mv->tamanioMemoria;

            InicializarRegistros(mv->registros);
        }
        else if (mv->versionVMX == 2){


            fread(tamanios, sizeof(char), 2, arch);
            high = tamanios[0] & 0x0FF;
            low = tamanios[1] & 0x0FF;
            tamanioCS = ((high << 8) | low);

            uint16_t tamaniosSeg[5]; // CS, DS, ES, SS, KS
            tamaniosSeg[0] = tamanioCS;

            for (int i = 1; i < 5; i++)
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


                mv->memoria = (char *)malloc(mv->tamanioMemoria);
                if (mv->memoria == NULL) {
                    fprintf(stderr, "ERROR: no se pudo asignar memoria\n");
                    //fclose(arch);
                    exit(1);
                }
                crearParamSegment(mv, parametros);


            /*mv->memoria = (char *)malloc(mv->tamanioMemoria);
            if (mv->memoria == NULL) {
                fprintf(stderr, "ERROR: no se pudo asignar memoria\n");
                fclose(arch);
                exit(1);
            }*/

            iniciarTablaSegmentos(mv, tamaniosSeg, 5);

            unsigned char entryPoint[2];
            if (fread(entryPoint, sizeof(char), 2, arch) != 2) {
                printf("ERROR: no se pudo leer el entry point\n");
                exit(1);
            }
            unsigned int tamanioEntryPoint = (entryPoint[0] << 8) | entryPoint[1];

            uint32_t direc = getDireccionFisica(*mv,mv->registros[CS]);

            for (int i = direc; i < direc + tamaniosSeg[0]; i++){
                fread(&mv->memoria[i], 1, 1, arch);
            }

            if (mv->registros[KS] != -1){
                direc = getDireccionFisica(*mv,mv->registros[KS]);
                for (int i = direc; i < direc + tamaniosSeg[4]; i++){
                    fread(&mv->memoria[i], 1, 1, arch);
                }
            }
                            mv->registros[IP] = mv->registros[CS];

        }
        else {
            printf("ERROR: versión de archivo incorrecta (%d)\n", mv->versionVMX);
            exit(1);
        }
        mv->breakpointFlag = 0;
        fclose(arch);
        return 1;
    }
}


void leerVMI(tipoMV *mv, char *fileName) {
    FILE *arch = fopen(fileName, "rb");
    unsigned char header[6], version;

    if (arch == NULL)
        printf("Error al abrir el archivo %s : ", fileName);
    else {
        fread(header, sizeof(char), 5, arch);
        header[5] = '\0';

        if (strcmp(header, IDENTIFICADOR_VMI) != 0) {
            printf("Archivo no valido: identificador incorrecto (%s)\n", header);
            exit(1);
        }

        fread(&version, sizeof(char), 1, arch);

        if (version != 0x01) {
            printf("Error: versión de archivo incorrecta (%d)\n", version);
            exit(1);
        }
        else
        {
            mv->versionVMI = 1;
            unsigned char tamMemoria[2];
            fread(tamMemoria, 1, 2, arch);
            mv->tamanioMemoria = (tamMemoria[0] << 8) | tamMemoria[1];
            printf("Tamaño de memoria VMI: %d bytes\n", mv->tamanioMemoria);
            mv->memoria = (char *)malloc(mv->tamanioMemoria);
            if (mv->memoria == NULL) {
                fprintf(stderr, "ERROR: no se pudo asignar memoria\n");
                fclose(arch);
                exit(1);
            }
            // fread(mv->registros, sizeof(uint32_t), NUM_REGISTROS, arch);
            for(int i = 0; i < NUM_REGISTROS; i++){
                uint8_t t_registro[4];
                fread(t_registro, 1, 4, arch);
                mv->registros[i] = (t_registro[0]<<24) | (t_registro[1]<<16) | (t_registro[2]<<8) | t_registro[3];
            }

             for(int i = 0; i < 8; i++){
                uint8_t t_segmento[4];
                fread(t_segmento, 1, 4, arch);
                mv->TS[i][0] = (t_segmento[0]<<8) | t_segmento[1];
                mv->TS[i][1] = (t_segmento[2]<<8) | t_segmento[3];
            }


            fread(mv->memoria, 1, mv->tamanioMemoria, arch);

        }
        fclose(arch);
    }
}

void crearVMI(tipoMV *mv, char *fileName) {
    FILE *arch;
    unsigned char header[6] = "VMI25";
    unsigned char version = 1;
    if ((arch = fopen(fileName, "wb")) == NULL)
        printf("Error al crear el archivo %s : ", fileName);
    else {

        fwrite(header, sizeof(char), 5, arch);
        fwrite(&version, sizeof(char), 1, arch);

        char tamMemoria[] = {(mv->tamanioMemoria & 0x0000FF00) >> 8, mv->tamanioMemoria & 0x000000FF};
        fwrite(tamMemoria, 1, 2, arch);
        // fwrite(mv->registros, sizeof(uint32_t), NUM_REGISTROS, arch);
        for(int i = 0; i < NUM_REGISTROS; i++){
            char registro[] = {(mv->registros[i]>>24)&0xFF,
                                (mv->registros[i]>>16)&0xFF,
                                (mv->registros[i]>>8)&0xFF,
                                (mv->registros[i])&0xFF};
            fwrite(registro, 1, 4, arch);
        }

        for(int i = 0; i < 8; i++){
            char segmento[] = {(mv->TS[i][0]>>8)&0xFF,
                                mv->TS[i][0] & 0xFF,
                                (mv->TS[i][1]>>8)&0xFF,
                                mv->TS[i][1] & 0xFF};
            fwrite(segmento, 1, 4, arch);
        }

        // for (int i = 0; i < 8; i++) {
        //     fwrite(&mv->TS[i][0], sizeof(uint16_t), 1, arch);
        //     fwrite(&mv->TS[i][1], sizeof(uint16_t), 1, arch);
        // }

        fwrite(mv->memoria, 1, mv->tamanioMemoria, arch);
        fclose(arch);
    }
}


void iniciarTablaSegmentos(tipoMV *mv, uint16_t sizes[], unsigned short int cantSegments) {
    // sizes = {CS, DS, ES, SS, KS}
    unsigned int base = 0;
    int indiceTS = -1;

    mv->registros[DS] = -1;
    mv->registros[ES] = -1;
    mv->registros[KS] = -1;

    if(mv->registros[PS] != -1) {
        indiceTS++;
        base += mv->TS[indiceTS][1];
    }

    //  Primero el segmento de constantes si existe
    if(sizes[cantSegments - 1] > 0) {
        indiceTS++;
        mv->TS[indiceTS][0] = base;
        mv->TS[indiceTS][1] = sizes[cantSegments - 1];
        mv->registros[KS] = (indiceTS << 16);
        base += mv->TS[indiceTS][1];
    }

    //  Luego los demás (CS, DS, ES, SS)
    for (int i = 0; i < 4; i++) {
        if (sizes[i] > 0) {
            indiceTS++;
            switch (i) {
                case 0: mv->registros[CS] = (indiceTS << 16);
                    break;
                case 1: mv->registros[DS] = (indiceTS << 16);
                    break;
                case 2: mv->registros[ES] = (indiceTS << 16);
                    break;
                case 3: mv->registros[SS] = (indiceTS << 16);
                        mv->registros[SP] = mv->registros[SS] + sizes[i];
                    break;
            }
            mv->TS[indiceTS][0] = base; // tamaño
            mv->TS[indiceTS][1] = sizes[i]; // base acumulada
            base += sizes[i];
        }
    }
}


void Disassembler(tipoMV programa)
{
    uint32_t direcBase = getDireccionFisica(programa, programa.registros[CS]);
    uint32_t dir = direcBase;
    uint32_t cantbytes;
    uint32_t op1, op2;

    while (dir < programa.TS[programa.registros[CS] >> 16][1] + direcBase)
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
            if ((op2 >> 24) != 0){
                PrintOperando(op2);
            }
        }
        printf("\n");
    }
}

void PrintOperando(uint32_t op)
{
    char registro[10];
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
                case 3: switch ((op & 0xC00000) >> 22){
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
    uint32_t dir = getDireccionFisica(*mv,mv->registros[IP]); // Dirección actual de instrucción

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



    if (mv->versionVMX == 2){
        if (mv->registros[PS]!= -1){
            pushearValor(mv,mv->argv);
            pushearValor(mv,mv->argc);
        }
        pushearValor(mv,-1);
    }


    while ((mv->registros[IP] < (( mv->registros[CS] + mv->TS[mv->registros[IP] >> 16][1] ))) && (mv->registros[IP] != -1))
    {
        /*printf("\n--- Estado de la Maquina Virtual ---\n");
        printf("IP: %08X\n", mv->registros[IP]);*/
        char opcionDebug;
        if (mv->breakpointFlag == 1)
        {
            scanf("%c", &opcionDebug);
            getchar();
            switch (opcionDebug)
            {
            case 'g':
            {
                mv->breakpointFlag = 0;
                break;
            }
            case 'q':
            {
                exit(0);
                break;
            }
            default:
            {
                breakpoint(mv);
                break;
            }
            }
        }

        // LECTURA INSTRUCCION
        uint32_t posicion = getDireccionFisica(*mv,mv->registros[IP]);
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
        printf("Fallo de segmento: segmento %d invalido\n", segmento);
        exit(1);
    }

    uint32_t base  = programa.TS[segmento][0];
    uint32_t limite = programa.TS[segmento][1];

    // Valida que el offset esté dentro del límite
    if (offset > limite) {
        printf("Fallo de segmento: offset 0x%X fuera del limite 0x%X del segmento %u\n",
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
        if ((programa->versionVMX == 2) || (programa->versionVMI == 1))
           cant_bytes -= ((OP & 0xC00000) >> 22);

        if ((OP & 0x1F0000) >> 16){
            if (OP & 0x8000)
                direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16] - CambiarSigno((OP & 0xFFFF) + 0xFFFF0000));
            else
                direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16]+ (OP & 0xFFFF));

        }
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
            if ((programa->versionVMX == 2) || (programa->versionVMI == 1)){
                uint32_t res;
                switch ((OP & 0xC0) >> 6){
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
        if ((programa->versionVMX == 2) || (programa->versionVMI == 1))
           cant_bytes -= ((OP & 0xC00000) >> 22);

        if ((OP & 0x1F0000) >> 16){

            if (OP & 0x8000)
                direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16] - CambiarSigno((OP & 0xFFFF) + 0xFFFF0000));
            else
                direccion_fisica = getDireccionFisica(*programa, programa->registros[(OP & 0x1F0000) >> 16]+ (OP & 0xFFFF));
        }
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (OP & 0xFFFF));
        SetearAccesoMemoria(programa, OP, cant_bytes, direccion_fisica);
        programa->registros[MBR] = valor_cargar;

        for (int i=0; i<cant_bytes ; i++){
            programa->memoria[direccion_fisica + cant_bytes - 1 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
    }
    else {
        if ((programa->versionVMX == 2) || (programa->versionVMI == 1)){
            switch ((OP & 0xC0)>>6){
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
    if ((programa->registros[SP] & 0xFFFF) < 0){
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

const char *NombreRegistro[32] = {
    [0] = "LAR",
    [1] = "MAR",
    [2] = "MBR",

    [3] = "IP",
    [4] = "OPC",
    [5] = "OP1",
    [6] = "OP2",

    [7] = "SP",
    [8] = "BP",

    [10] = "EAX",
    [11] = "EBX",
    [12] = "ECX",
    [13] = "EDX",
    [14] = "EEX",
    [15] = "EFX",

    [16] = "AC",
    [17] = "CC",

    [26] = "CS",
    [27] = "DS",
    [28] = "ES",
    [29] = "SS",
    [30] = "KS",
    [31] = "PS"
};
