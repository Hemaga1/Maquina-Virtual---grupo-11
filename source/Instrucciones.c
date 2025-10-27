#include "MaquinaVirtual.h"
#include "Instrucciones.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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
    operacion[11] = &PUSH;
    operacion[12] = &POP;
    operacion[13] = &CALL;
    operacion[14] = &RET;

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
    [11] = "PUSH",
    [12] = "POP",
    [13] = "CALL",
    [14] = "RET",

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

    if (dividir != 0){

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
        printf("ERROR: Division por 0.\n");
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

    if ((programa->registros[OP1] & 0xFFFF) == 1){
            char cadena[33];
            uint32_t direccion_fisica = getDireccionFisica(*programa,programa->registros[EDX]);
            uint16_t formato = programa->registros[EAX];

        for (int i=0;i<(programa->registros[ECX] & 0xFFFF);i++){

            formato = programa->registros[EAX];

            SetearAccesoMemoria(programa, (13 << 16) + (programa->registros[EDX] & 0xFFFF), (programa->registros[ECX] >> 16), direccion_fisica);

            programa->registros[MBR] = 0;

            printf(" [%04X] ",direccion_fisica);


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

                for (int j=0; j<(programa->registros[ECX] >> 16); j++){
                    programa->memoria[direccion_fisica + (programa->registros[ECX] >> 16) - 1 - j] = ((programa->registros[MBR] & (0xFF << (j*8))) >> (j*8));
                }

            }
            direccion_fisica += (programa->registros[ECX] >> 16);
        }
        printf("\n");
    }
    else if ((programa->registros[OP1] & 0xFFFF) == 2){
            char cadena[33];
            uint32_t direccion_fisica = getDireccionFisica(*programa,programa->registros[EDX]);
            uint16_t formato = programa->registros[EAX];
            programa->registros[MBR] = 0;

        for (int i=0; i<(programa->registros[ECX] & 0xFFFF); i++){

            formato = programa->registros[EAX];

            SetearAccesoMemoria(programa, (13 << 16) + (programa->registros[EDX] & 0xFFFF),(programa->registros[ECX] >> 16), direccion_fisica);

            programa->registros[MBR] = 0;

            for (int j = 0; j < (programa->registros[ECX] >> 16); j++) {
                programa->registros[MBR] += (programa->memoria[direccion_fisica + (programa->registros[ECX] >> 16) - 1 - j] << (j * 8));
            }


            printf(" [%04X] ",direccion_fisica);

            for (int k=4;k>-1;k--){
                if (formato & 0x10)
                    switch (k){
                        case 0: if (programa->registros[MBR] & 0x80000000)
                                    printf("%d ",(int32_t)programa->registros[MBR]);
                                else
                                    printf("%d ",programa->registros[MBR]);
                                break;
                        case 1:
                                for (int g =(programa->registros[ECX] >> 16) - 1; g > -1 ; g--){
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

            direccion_fisica += (programa->registros[ECX] >> 16);
        }
    }
    else if ((programa->registros[OP1] & 0xFFFF) == 3) {
            uint32_t direccion_fisica = getDireccionFisica(*programa,programa->registros[EDX]);
            uint16_t formato = programa->registros[EAX];

            printf(" [%04X] ",direccion_fisica);

            char cadena[1000];
            fgets(cadena,1000,stdin);
            int i;
            if ((programa->registros[ECX] & 0xFFFF) != 0xFFFF){
                for (i=0; i<programa->registros[ECX]; i++){
                    programa->registros[MBR] = cadena[i];
                    SetearAccesoMemoria(programa, (13 << 16) + (programa->registros[EDX] & 0xFFFF) + i, 1 , direccion_fisica + i);
                    programa->memoria[direccion_fisica + i] = cadena[i];
                }
            }
            else {
                int i=0;
                while (cadena[i] != '\0'){
                    programa->registros[MBR] = cadena[i];
                    SetearAccesoMemoria(programa, (0x13 << 16) + (programa->registros[EDX] & 0xFFFF) + i, 1 , direccion_fisica + i);
                    programa->memoria[direccion_fisica + i] = cadena[i];
                    i++;
                }
            }
            programa->memoria[direccion_fisica + i] = '\0';
    }
    else if ((programa->registros[OP1] & 0xFFFF) == 4) {

            uint32_t direccion_fisica = getDireccionFisica(*programa,programa->registros[EDX]);
            uint16_t formato = programa->registros[EAX];

            printf(" [%04X] ",direccion_fisica);



            char caracter = ' ';
            SetearAccesoMemoria(programa, (13 << 16) + (programa->registros[EDX] & 0xFFFF), 1 , direccion_fisica);
            caracter = programa->memoria[direccion_fisica];
            int i=1;
            while ((caracter != '\0') && (i < programa->registros[ECX])){
                programa->registros[MBR] = caracter;
                if ( (caracter > 0) && (caracter<255) && isprint(caracter))
                    printf("%c",caracter);
                else
                    printf("%c",'.');
                SetearAccesoMemoria(programa, (13 << 16) + (programa->registros[EDX] & 0xFFFF) + i, 1 , direccion_fisica + i);
                caracter = programa->memoria[direccion_fisica + i];
                i++;
            }
            printf("\n");
    }
    else if ((programa->registros[OP1] & 0xFFFF) == 7){
                system("cls");
    }
    else if ((programa->registros[OP1] & 0xFFFF) == 15) {
                breakpoint(programa);
    }
}

void breakpoint(tipoMV *mv){
    if (mv->nombreVMI == NULL){
        mv->nombreVMI = mv->nombreVMX;
        mv->nombreVMI[strlen(mv->nombreVMI)-1] = 'i';
    }
    crearVMI(mv, mv->nombreVMI);
    mv->breakpointFlag = 1;
}

void JMP(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    programa->registros[OP1] = programa->registros[OP2];
    programa->registros[OP2] = 0;

    valor_cargar = getValorCargar(programa, op2);

    ModificarIP(programa, programa->registros[CS] + valor_cargar);
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

void PUSH(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t valor_cargar;
    uint32_t direccion_fisica;
    programa->registros[OP1] = programa->registros[OP2];
    programa->registros[OP2] = 0;

    programa->registros[SP] -= 4;
    if (programa->registros[SP] < programa->registros[SS]){
        printf("ERROR: STACK OVERFLOW.\n");
        exit(1);
    }
    else {
        valor_cargar = getValorCargar(programa, op2);
        direccion_fisica = getDireccionFisica(*programa,  programa->registros[SP]);

        programa->registros[LAR] = programa->registros[SP];
        programa->registros[MAR] = direccion_fisica;
        programa->registros[MAR] = (programa->registros[MAR] & 0xFFFF) + (4 << 16);
        programa->registros[MBR] = valor_cargar;

        for (int i=0; i<4 ; i++){
            programa->memoria[direccion_fisica + 3 - i] = (programa->registros[MBR] & (0xFF << (i*8))) >> (i*8);
        }
    }
}

void POP(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t direccion_fisica;
    programa->registros[OP1] = programa->registros[OP2];
    programa->registros[OP2] = 0;

    direccion_fisica = getDireccionFisica(*programa,  programa->registros[SP]);

    programa->registros[MBR] = 0;
    for (int i=0; i<4; i++){
        programa->registros[MBR] |= programa->memoria[direccion_fisica + 3 - i] << (i*8);
    }

    setOperando(programa,op2,programa->registros[MBR]);

    programa->registros[SP] += 4;

    if ((programa->registros[SP] & 0xFFFF) > programa->TS[programa->registros[SS] >> 16][1]){
        printf("ERROR: STACK UNDERFLOW.\n");
        exit(1);
    }

}

void CALL(tipoMV *programa, uint32_t op1, uint32_t op2){
    pushearValor(programa,programa->registros[IP]);

    JMP(programa,op1,op2);
}

void RET(tipoMV *programa, uint32_t op1, uint32_t op2){
    uint32_t direccion_fisica;
    programa->registros[OP1] = programa->registros[OP2];
    programa->registros[OP2] = 0;

    direccion_fisica = getDireccionFisica(*programa,  programa->registros[SP]);

    programa->registros[MBR] = 0;
    for (int i=0; i<4; i++){
        programa->registros[MBR] |= programa->memoria[direccion_fisica + 3 - i] << (i*8);
    }


    programa->registros[IP] = programa->registros[MBR];

    programa->registros[SP] += 4;


    if ((programa->registros[SP] & 0xFFFF) > programa->TS[programa->registros[SS] >> 16][1]){
        printf("ERROR: STACK UNDERFLOW.\n");
        exit(1);
    }
}

//Sin operandos

void STOP(tipoMV *programa, uint32_t op1, uint32_t op2){
    programa->registros[IP] = -1;
}
