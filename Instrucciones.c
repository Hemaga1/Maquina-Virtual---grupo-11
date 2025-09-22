#include "MaquinaVirtual.h"
#include "Instrucciones.h"
#include <stdio.h>


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
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = valor_cargar;

        for (int i=0; i<4 ; i++){
            programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            programa->registros[op1 & 0x1F] = valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }

}

void ADD(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
            programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }
        if (valor_cargar >= 0)
            programa->registros[MBR] += valor_cargar;
        else
            programa->registros[MBR] -= CambiarSigno(valor_cargar);

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
            programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            if (valor_cargar & 0x80000000)
                programa->registros[op1 & 0x1F] -= CambiarSigno(valor_cargar) ;
            else
                programa->registros[op1 & 0x1F] += valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void SUB(tipoMV *programa , uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
            programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        if (valor_cargar >= 0)
            programa->registros[MBR] -= valor_cargar;
        else
            programa->registros[MBR] += CambiarSigno(valor_cargar);

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
            programa->memoria[direccion_fisica + 4 -i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            if (valor_cargar & 0x80000000)
                programa->registros[op1 & 0x1F] += CambiarSigno(valor_cargar) ;
            else
                programa->registros[op1 & 0x1F] -= valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void MUL(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint8_t negativo = 0;
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if(valor_cargar & 0x80000000){
        negativo = 1;
        valor_cargar = CambiarSigno(valor_cargar);
    }

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
            programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }

        if (programa->registros[MBR] & 0x80000000){
            if (negativo)
                    negativo = 0;
                else
                    negativo = 1;
            programa->registros[MBR] = CambiarSigno(programa->registros[MBR]);
        }

        programa->registros[MBR] *= valor_cargar;

        if (negativo) {
            programa->registros[MBR] = CambiarSigno(programa->registros[MBR]);
        }

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
            programa->memoria[direccion_fisica + 4 - i] = programa->registros[MBR] & (0xFF << (i*8));
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            if (programa->registros[op1 & 0x1F] & 0x80000000){
                if (negativo)
                    negativo = 0;
                else
                    negativo = 1;
            programa->registros[op1 & 0x1F] = CambiarSigno(programa->registros[op1 & 0x1F]);
            }

            programa->registros[op1 & 0x1F] *= valor_cargar;

            if (negativo){
                programa->registros[op1 & 0x1F] = CambiarSigno(programa->registros[op1 & 0x1F]);
            }
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void DIV(tipoMV *programa , uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint8_t negativo = 0;
    uint32_t direccion_fisica;
    uint32_t valor_cargar;
    uint32_t valor_original;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);


    if (valor_cargar != 0){

        if(valor_cargar & 0x80000000){
            negativo = 1;
            valor_cargar = CambiarSigno(valor_cargar);
        }


        if (tipo_op1 == 3){
            if (programa->registros[(op1 & 0x1F0000) >> 16])
                direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
            else
                direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
            SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
            programa->registros[MBR] = 0;

            for (int i=0; i<4; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
            }

            valor_original = programa->registros[MBR];

            if (programa->registros[MBR] & 0x80000000){
                if (negativo)
                        negativo = 0;
                    else
                        negativo = 1;
                programa->registros[MBR] = CambiarSigno(programa->registros[MBR]);
            }


            programa->registros[MBR] = programa->registros[MBR] / valor_cargar;

            if (negativo) {
                programa->registros[MBR] = CambiarSigno(programa->registros[MBR]);
            }

            SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
            for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
            }
            ModificarCC(programa,programa->registros[MBR]);
            programa->registros[AC] = valor_original - (valor_cargar*programa->registros[MBR]);
        }
        else {
                if (programa->registros[op1 & 0x1F] & 0x80000000){
                    if (negativo)
                        negativo = 0;
                    else
                        negativo = 1;
                programa->registros[op1 & 0x1F] = CambiarSigno(programa->registros[op1 & 0x1F]);
                }

                valor_original = programa->registros[op1 & 0x1F];

                programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] / valor_cargar;

                if (negativo){
                    programa->registros[op1 & 0x1F] = CambiarSigno(programa->registros[op1 & 0x1F]);
                }
                ModificarCC(programa,programa->registros[op1 & 0x1F]);
                programa->registros[AC] = valor_original - (valor_cargar*programa->registros[op1 & 0x1F]);
        }
    }
    else {
        printf("ERROR: Division por 0\n");
        exit(1);
    }

}

void CMP(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;

        for (int i=0; i<4; i++){
                programa->registros[MBR] += (programa->memoria[direccion_fisica + 4 - i] << (i*8));
        }

        if (valor_cargar & 0x80000000)
            ModificarCC(programa,programa->registros[MBR] + CambiarSigno(valor_cargar));
        else
            ModificarCC(programa,programa->registros[MBR] - valor_cargar);
    }
    else {
            if (valor_cargar & 0x80000000)
                ModificarCC(programa,programa->registros[op1 & 0x1F] + CambiarSigno(valor_cargar));
            else
                ModificarCC(programa,programa->registros[op1 & 0x1F] - valor_cargar);
    }
}

void SHL(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }
        programa->registros[MBR] = programa->registros[MBR] << valor_cargar;

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] << valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void SHR(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }
        programa->registros[MBR] = programa->registros[MBR] >> valor_cargar;

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica-i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] >> valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void SAR(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }
        if (programa->registros[MBR] & 0x80000000)
            for (int i=0;i<valor_cargar;i++){
                programa->registros[MBR] = programa->registros[MBR] >> 1;
                programa->registros[MBR] += (1 << 31);
            }
        else programa->registros[MBR] = programa->registros[MBR] >> valor_cargar;

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            if (programa->registros[op1 & 0x1F] & 0x80000000)
                for (int i=0;i<valor_cargar;i++){
                    programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] >> 1;
                    programa->registros[op1 & 0x1F] += (1 << 31);
                }
            else programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] >> valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void AND(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }
        programa->registros[MBR] = programa->registros[MBR] & valor_cargar;

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] & valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void OR(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }
        programa->registros[MBR] = programa->registros[MBR] | valor_cargar;

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] | valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void XOR(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }
        programa->registros[MBR] = programa->registros[MBR] ^ valor_cargar;

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] ^ valor_cargar;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

void SWAP(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargarOP1;
    uint32_t valor_cargarOP2;

    valor_cargarOP1 = getValorCargar(programa, op2, tipo_op2, 4);
    valor_cargarOP2 = getValorCargar(programa, op1, tipo_op1, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        programa->registros[MBR] = valor_cargarOP1;
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
    }
    else {
            programa->registros[op1 & 0x1F] = valor_cargarOP1;
    }

    if (tipo_op2 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op2 & 0x1F0000) >> 16]+ (op2 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op2 & 0xFFFF));
        programa->registros[MBR] = valor_cargarOP2;
        SetearAccesoMemoria(programa, op2, 1, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
    }
    else {
            programa->registros[op2 & 0x1F] = valor_cargarOP1;
    }
}

void LDH(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 2, direccion_fisica+2);
        programa->registros[MBR] = valor_cargar;
        /*for (int i=0; i<2; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 4 - i] << (i*8);
        }*/


        //programa->registros[MBR] = (programa->memoria[direccion_fisica] & 0xFFFF) + (valor_cargar << 16);
        SetearAccesoMemoria(programa, op1, 2, direccion_fisica+2);
        for (int i=0; i<2; i++){
                programa->memoria[direccion_fisica + 2 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        //programa->memoria[direccion_fisica+2] = programa->registros[MBR];
    }
    else programa->registros[op1 & 0x1F] = (programa->registros[op1 & 0x1F] & 0xFFFF) + (valor_cargar << 16);
}

void LDL(tipoMV *programa, uint32_t op1, uint32_t op2){

    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;


    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 2, direccion_fisica);
        programa->registros[MBR] = valor_cargar;
        /*for (int i=0; i<2; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica + 2 - i] << (i*8);
        }*/
        //programa->registros[MBR] = valor_cargar + (programa->memoria[direccion_fisica] & 0xFFFF0000);
        SetearAccesoMemoria(programa, op1, 2, direccion_fisica);
        for (int i=0; i<2; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
    }
    else programa->registros[op1 & 0x1F] = (programa->registros[op1 & 0x1F] & 0xFFFF0000) + valor_cargar;
}

void RND(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint8_t tipo_op2 = getTipoOperando(op2);
    uint32_t direccion_fisica;
    int32_t valor_cargar;

    valor_cargar = getValorCargar(programa, op2, tipo_op2, 4);

    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->registros[MBR] += programa->memoria[direccion_fisica  + 4 - i] << (i*8);
        }
        programa->registros[MBR] = rand() % valor_cargar + 1;

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        for (int i=0; i<4; i++){
                programa->memoria[direccion_fisica + 4 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
            programa->registros[op1 & 0x1F] = rand() % valor_cargar + 1;
            ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
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
                    programa->memoria[direccion_fisica + ((programa->registros[ECX] & 0xFFFF0000) >> 16) - j] = ((programa->registros[MBR] & (0xFF << (j*8))) >> (j*8));
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
            for (int j=0; j<((programa->registros[ECX] & 0xFFFF0000) >> 16); j++){
                programa->registros[MBR] += (programa->memoria[direccion_fisica + ((programa->registros[ECX] & 0xFFFF0000) >> 16) - j] << (j*8));
            }

            for (int k=0;k<5;k++){
                if (formato & 1)
                    switch (k){
                        case 0: if (programa->registros[MBR] & 0x80000000)
                                    printf("[%04X]: -%d\n",direccion_fisica, CambiarSigno(programa->registros[MBR]));
                                else
                                    printf("[%04X]: %d\n",direccion_fisica, programa->registros[MBR]);
                        case 1:
                            if (programa->registros[MBR] < 255 && isprint(programa->registros[MBR]))
                                printf("[%X]: %c\n", direccion_fisica, programa->registros[MBR]);
                            else
                                printf("[%X]: .\n", direccion_fisica);
                            break;
                        case 2: printf("[%04X]: 0o%o\n",direccion_fisica, programa->registros[MBR]);
                            break;
                        case 3: printf("[%04X]: 0x%X\n",direccion_fisica, programa->registros[MBR]);
                            break;
                        case 4: printf("[%04X]: 0b",direccion_fisica); MostrarBinario(programa->registros[MBR]); printf("\n");
                            break;
                    }
                formato = (formato >> 1);
            }
            direccion_fisica += ((programa->registros[ECX] & 0xFFFF0000) >> 16);
        }
    }
}

void JMP(tipoMV *programa, uint32_t op1, uint32_t op2){
    programa->registros[OP1] = programa->registros[OP2];
    programa->registros[OP2] = 0;

    op1 = programa->registros[OP1];
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint32_t direccion_fisica;
    uint32_t valor_cargar;


    if (tipo_op1 == 3){
        if (programa->registros[(op1 & 0x1F0000) >> 16])
            direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        else
            direccion_fisica = getDireccionFisica(*programa,  0x10000 + (op1 & 0xFFFF));

        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = 0;
        for (int i=0; i<4; i++){
                programa->registros[MBR] += (programa->memoria[direccion_fisica - i] << (i*8));
        }
        valor_cargar = programa->registros[MBR] + 8;
    }
    else
        if (tipo_op1 == 1)
            valor_cargar = programa->registros[op1 & 0x1F];
        else{
            if (tipo_op1 == 2){
                if (op1 & 0xFFFF)
                    valor_cargar = (op1 & 0xFFFF);
                else
                    valor_cargar = 0;
            }
        }
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
    programa->registros[OP1] = programa->registros[OP2];
    programa->registros[OP2] = 0;

    op1 = programa->registros[OP1];
    uint8_t tipo_op1 = getTipoOperando(op1);
    uint32_t direccion_fisica;

    if (tipo_op1 == 3){
        direccion_fisica = getDireccionFisica(*programa, programa->registros[(op1 & 0x1F0000) >> 16]+ (op1 & 0xFFFF));
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->registros[MBR] = programa->memoria[direccion_fisica] ^ 0xFFFFFFFF;
        SetearAccesoMemoria(programa, op1, 4, direccion_fisica);
        programa->memoria[direccion_fisica] = programa->registros[MBR];
        ModificarCC(programa,programa->registros[MBR]);
    }
    else {
        programa->registros[op1 & 0x1F] = programa->registros[op1 & 0x1F] ^ 0xFFFFFFFF;
        ModificarCC(programa,programa->registros[op1 & 0x1F]);
    }
}

//Sin operandos

void STOP(tipoMV *programa, uint32_t op1, uint32_t op2){
    programa->registros[IP] = -1;
}
