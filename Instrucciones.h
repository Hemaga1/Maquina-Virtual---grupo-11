
#ifndef INSTRUCCIONES_H_INCLUDED
#define INSTRUCCIONES_H_INCLUDED

#include "maquinaVirtual.h"

#define NUM_INSTRUCCIONES 32

typedef void (*funcion)(tipoMV *programa, uint32_t op1, uint32_t op2);

void inicioVectorOper(funcion operacion[]);
extern const char* Mnemonicos[32];

void MOV(tipoMV *programa, uint32_t op1, uint32_t op2);
void ADD(tipoMV *programa, uint32_t op1, uint32_t op2);
void SUB(tipoMV *programa, uint32_t op1, uint32_t op2);
void MUL(tipoMV *programa, uint32_t op1, uint32_t op2);
void DIV(tipoMV *programa, uint32_t op1, uint32_t op2);
void CMP(tipoMV *programa, uint32_t op1, uint32_t op2);
void SHL(tipoMV *programa, uint32_t op1, uint32_t op2);
void SHR(tipoMV *programa, uint32_t op1, uint32_t op2);
void SAR(tipoMV *programa, uint32_t op1, uint32_t op2);
void AND(tipoMV *programa, uint32_t op1, uint32_t op2);
void OR(tipoMV *programa, uint32_t op1, uint32_t op2);
void XOR(tipoMV *programa, uint32_t op1, uint32_t op2);
void SWAP(tipoMV *programa, uint32_t op1, uint32_t op2);
void LDL(tipoMV *programa, uint32_t op1, uint32_t op2);
void LDH(tipoMV *programa, uint32_t op1, uint32_t op2);
void RND(tipoMV *programa, uint32_t op1, uint32_t op2);
void SYS(tipoMV *programa, uint32_t op1, uint32_t op2);
void JMP(tipoMV *programa, uint32_t op1, uint32_t op2);
void JZ(tipoMV *programa, uint32_t op1, uint32_t op2);
void JP(tipoMV *programa, uint32_t op1, uint32_t op2);
void JN(tipoMV *programa, uint32_t op1, uint32_t op2);
void JNZ(tipoMV *programa, uint32_t op1, uint32_t op2);
void JNP(tipoMV *programa, uint32_t op1, uint32_t op2);
void JNN(tipoMV *programa, uint32_t op1, uint32_t op2);
void NOT(tipoMV *programa, uint32_t op1, uint32_t op2);
void STOP(tipoMV *programa, uint32_t op1, uint32_t op2);


#endif // INSTRUCCIONES_H_INCLUDED
