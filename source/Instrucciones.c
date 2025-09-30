#include "MaquinaVirtual.h"
#include "Instrucciones.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

void inicioVectorOper(funcion operacion[]) {
    operacion[0]  = &SYS;
    operacion[1]  = &JMP;
    operacion[2]  = &JZ;
    operacion[3]  = &JP;
    operacion[4]  = &JN;
    operacion[5]  = &JNZ;
    operacion[6]  = &JNP;
    operacion[7]  = &JNN;
    operacion[8]  = &NOT;

    // Posiciones no utilizadas (instrucciones invalidas)
    operacion[9]  = NULL;
    operacion[10] = NULL;
    operacion[11] = NULL;
    operacion[12] = NULL;
    operacion[13] = NULL;
    operacion[14] = NULL;

    operacion[15] = &STOP;

    operacion[16] = &MOV;
    operacion[17] = &ADD;
    operacion[18] = &SUB;
    operacion[19] = &MUL;
    operacion[20] = &DIV;
    operacion[21] = &CMP;
    operacion[22] = &SHL;
    operacion[23] = &SHR;
    operacion[24] = &SAR;
    operacion[25] = &AND;
    operacion[26] = &OR;
    operacion[27] = &XOR;
    operacion[28] = &SWAP;
    operacion[29] = &LDL;
    operacion[30] = &LDH;
    operacion[31] = &RND;
}


const char *Mnemonicos[32] = {
    [0] = "SYS",
    [1] = "JMP",
    [2] = "JZ",
    [3] = "JP",
    [4] = "JN",
    [5] = "JNZ",
    [6] = "JNP",
    [7] = "JNN",
    [8] = "NOT",

    [9]  = "Codigo de instruccion Invalido",
    [10] = "Codigo de instruccion Invalido",
    [11] = "Codigo de instruccion Invalido",
    [12] = "Codigo de instruccion Invalido",
    [13] = "Codigo de instruccion Invalido",
    [14] = "Codigo de instruccion Invalido",

    [15] = "STOP",

    [16] = "MOV",
    [17] = "ADD",
    [18] = "SUB",
    [19] = "MUL",
    [20] = "DIV",
    [21] = "CMP",
    [22] = "SHL",
    [23] = "SHR",
    [24] = "SAR",
    [25] = "AND",
    [26] = "OR",
    [27] = "XOR",
    [28] = "SWAP",
    [29] = "LDL",
    [30] = "LDH",
    [31] = "RND"
};

void MOV(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2);
    setOperando(programa,op1,valor_cargar);
}

void ADD(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t sumar;

    sumar = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    if (sumar & 0x80000000)
        valor_cargar -= CambiarSigno(sumar);
    else
        valor_cargar += sumar;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void SUB(tipoMV *programa , uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t restar;

    restar = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    if (restar & 0x80000000)
        valor_cargar += CambiarSigno(restar);
    else
        valor_cargar -= restar;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void MUL(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t multiplicar;
    uint8_t negativo = 0;

    multiplicar = getValorCargar(programa, op2);

    if(multiplicar & 0x80000000){
        negativo = 1;
        multiplicar = CambiarSigno(multiplicar);
    }

    valor_cargar = getValorCargar(programa, op1);

    if(valor_cargar & 0x80000000){
        if (negativo)
                    negativo = 0;
                else
                    negativo = 1;
        valor_cargar = CambiarSigno(valor_cargar);
    }

    valor_cargar *= multiplicar;

    if (negativo) {
        valor_cargar = CambiarSigno(valor_cargar);
    }

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void DIV(tipoMV *programa , uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t valor_original;
    uint32_t dividir;
    uint8_t negativo = 0;

    dividir = getValorCargar(programa, op2);

    if (valor_cargar != 0){

        if(dividir & 0x80000000){
            negativo = 1;
            dividir= CambiarSigno(dividir);
        }

        valor_cargar = getValorCargar(programa, op1);
        valor_original = valor_cargar;

        if(valor_cargar & 0x80000000){
            if (negativo)
                    negativo = 0;
                else
                    negativo = 1;
            valor_cargar = CambiarSigno(valor_cargar);
        }

        valor_cargar /= dividir;

        if (negativo) {
            valor_cargar = CambiarSigno(valor_cargar);
        }

        setOperando(programa,op1,valor_cargar);

        ModificarCC(programa,valor_cargar);
        programa->registros[AC] = valor_original - (valor_cargar*dividir);

    }
    else {
        printf("ERROR: Division por 0\n");
        exit(1);
    }
}

void CMP(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t restar;

    restar = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    if (restar & 0x80000000)
        ModificarCC(programa,valor_cargar + CambiarSigno(restar));
    else
        ModificarCC(programa,valor_cargar - restar);
}

void SHL(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t shift;

    shift = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    valor_cargar = valor_cargar << shift;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void SHR(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t shift;

    shift = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    valor_cargar = valor_cargar >> shift;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void SAR(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t shift;

    shift = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    if (valor_cargar & 0x80000000)
        for (int i=0;i<shift;i++){
            valor_cargar = valor_cargar >> 1;
            valor_cargar += (1 << 31);
        }
    else valor_cargar = valor_cargar >> shift;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void AND(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t valor_and;

    valor_and = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    valor_cargar = valor_cargar & valor_and;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void OR(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t valor_or;

    valor_or = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    valor_cargar = valor_cargar | valor_or;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void XOR(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t valor_xor;

    valor_xor = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    valor_cargar = valor_cargar ^ valor_xor;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

void SWAP(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargarOP1;
    uint32_t valor_cargarOP2;

    valor_cargarOP1 = getValorCargar(programa, op2);
    valor_cargarOP2 = getValorCargar(programa, op1);

    setOperando(programa,op1,valor_cargarOP1);
    setOperando(programa,op2,valor_cargarOP2);
}

void LDH(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t valor_ldh;

    valor_ldh = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    valor_cargar = (valor_cargar & 0xFFFF) + (valor_ldh << 16);

    setOperando(programa,op1,valor_cargar);
}

void LDL(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t valor_ldh;

    valor_ldh = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    valor_cargar = (valor_cargar & 0xFFFF0000) + valor_ldh;

    setOperando(programa,op1,valor_cargar);
}

void RND(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t limite;

    srand(time(NULL));

    limite = getValorCargar(programa, op2);
    valor_cargar = getValorCargar(programa, op1);

    valor_cargar = rand() % (limite + 1);

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}


void SYS(tipoMV *programa, uint32_t op1, uint32_t op2){
    programa->registros[OP1] = op2;
    programa->registros[OP2] = 0;
    char cadena[33];
    uint32_t direccion_fisica = getDireccionFisica(*programa,programa->registros[EDX]);
    uint16_t formato = programa->registros[EAX];
    SetearAccesoMemoria(programa, (0x13 << 16) + (programa->registros[EDX] & 0xFFFF), (programa->registros[ECX] & 0xFFFF) , direccion_fisica);
    programa->registros[MBR] = 0;


    if (programa->registros[OP1] & 1){

        for (int i=0;i<(programa->registros[ECX] & 0xFFFF);i++){

            formato = programa->registros[EAX];

            SetearAccesoMemoria(programa, (0x13 << 16) + (programa->registros[EDX] & 0xFFFF), (programa->registros[ECX] & 0xFFFF) , direccion_fisica);

            programa->registros[MBR] = 0;

            printf("[%04X]: ",direccion_fisica);

            for (int k=0;k<5;k++){
                if (formato & 1)
                    switch (k){
                        case 0: scanf("%d",&programa->registros[MBR]);
                        break;
                        case 1: scanf("%c",&programa->registros[MBR]);
                            break;
                        case 2: scanf("%o",&programa->registros[MBR]);
                            break;
                        case 3: scanf("%X",&programa->registros[MBR]);
                            break;
                        case 4: scanf("%s",cadena); programa->registros[MBR] = StringABinario(cadena);
                            break;
                    }
                formato = formato >> 1;

                for (int j=0; j<((programa->registros[ECX] & 0xFFFF0000) >> 16); j++){
                    programa->memoria[direccion_fisica + ((programa->registros[ECX] & 0xFFFF0000) >> 16) - 1 - j] = ((programa->registros[MBR] & (0xFF << (j*8))) >> (j*8));
                }

            }
            direccion_fisica += ((programa->registros[ECX] & 0xFFFF0000) >> 16);
        }

    }
    else {
        for (int i=0; i<(programa->registros[ECX] & 0xFFFF); i++){

            formato = programa->registros[EAX];

            SetearAccesoMemoria(programa, (0x13 << 16) + (programa->registros[EDX] & 0xFFFF), (programa->registros[ECX] & 0xFFFF) , direccion_fisica);

            programa->registros[MBR] = 0;

            uint8_t size = (programa->registros[ECX] & 0xFFFF0000) >> 16;
            for (int j = 0; j < size; j++) {
                programa->registros[MBR] += (programa->memoria[direccion_fisica + size - 1 - j] << (j * 8));
            }


            printf("[%04X]: ",direccion_fisica);

            for (int k=4;k>-1;k--){
                if (formato & 0x10)
                    switch (k){
                        case 0: if (programa->registros[MBR] & 0x80000000)
                                    printf("%d\n",(int32_t)programa->registros[MBR]);
                                else
                                    printf("%d\n",(int32_t)programa->registros[MBR]);
                                break;
                        case 1:
                                for (int g=((programa->registros[ECX] & 0xFFFF0000) >> 16)-1; g>-1; g--){
                                    if (((programa->registros[MBR] & (0xFF << (g*8))) >> (g*8) < 255) && isprint((programa->registros[MBR] & (0xFF << (g*8))) >> (g*8)))
                                        printf("%c",(programa->registros[MBR] & (0xFF << (g*8))) >> (g*8));
                                    else {
                                        printf(".");
                                    }
                                }
                                printf(" ");
                            break;
                        case 2: printf("0o%o ",programa->registros[MBR]);
                            break;
                        case 3: printf("0x%X ",programa->registros[MBR]);
                            break;
                        case 4: printf("0b"); MostrarBinario(programa->registros[MBR]); printf(" ");
                            break;
                    }
                formato = (formato << 1);
            }

            printf("\n");

            direccion_fisica += ((programa->registros[ECX] & 0xFFFF0000) >> 16);
        }
    }
}

void JMP(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    programa->registros[OP1] = programa->registros[OP2];
    programa->registros[OP2] = 0;

    valor_cargar = getValorCargar(programa, op2);

    ModificarIP(programa, valor_cargar);
}

void JZ(tipoMV *programa, uint32_t op1, uint32_t op2){
    if ((programa->registros[CC] & (1 << 30)) != 0){
        JMP(programa, op1, op2);
    }
}

void JP(tipoMV *programa, uint32_t op1, uint32_t op2){
    if ((programa->registros[CC] & (1 << 31)) == 0 && (programa->registros[CC] & (1 << 30)) == 0){
        JMP(programa, op1, op2);
    }
}

void JN(tipoMV *programa, uint32_t op1, uint32_t op2){
    if ((programa->registros[CC] & (1 << 31)) != 0){
        JMP(programa, op1, op2);
    }
}

void JNZ(tipoMV *programa, uint32_t op1, uint32_t op2){
    if ((programa->registros[CC] & (1 << 30)) == 0){
        JMP(programa, op1, op2);
    }
}

void JNP(tipoMV *programa, uint32_t op1, uint32_t op2){
    if ((programa->registros[CC] & (1 << 31)) != 0 || (programa->registros[CC] & (1 << 30)) != 0){
        JMP(programa, op1, op2);
    }
}

void JNN(tipoMV *programa, uint32_t op1, uint32_t op2){
    if ((programa->registros[CC] & (1 << 31)) == 0){
        JMP(programa, op1, op2);
    }
}

void NOT(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    programa->registros[OP1] = programa->registros[OP2];
    programa->registros[OP2] = 0;

    valor_cargar = getValorCargar(programa, op2);

    valor_cargar = valor_cargar ^ 0xFFFFFFFF;

    setOperando(programa,op1,valor_cargar);

    ModificarCC(programa,valor_cargar);
}

//Sin operandos

void STOP(tipoMV *programa, uint32_t op1, uint32_t op2){
    programa->registros[IP] = -1;
}
