#ifndef MAQUINAVIRTUAL_H_INCLUDED
#define MAQUINAVIRTUAL_H_INCLUDED


#include <stdint.h>
#define TAMANIO_MEMORIA 16384
#define NUM_REGISTROS 32

#define VERSION 1
#define IDENTIFICADOR "VMX25"

#define TAM_CELDA 4


typedef enum
{
    // Acceso a memoria
    LAR = 0,
    MAR = 1,
    MBR = 2,

    // Instrucci�n
    IP = 3,
    OPC = 4,
    OP1 = 5,
    OP2 = 6,

    // Registros de prop�sito general
    EAX = 10,
    EBX = 11,
    ECX = 12,
    EDX = 13,
    EEX = 14,
    EFX = 15,

    // Acumulador y c�digo de condici�n
    AC = 16,
    CC = 17,

    // Segmentos
    CS = 26,
    DS = 27,
} t_registro;

typedef struct MV
{
    char memoria[TAMANIO_MEMORIA];
    uint32_t registros[NUM_REGISTROS];
    uint16_t TS[2][2];
    char *nombreVMX;
    char version;
} tipoMV;

typedef enum
{
    NINGUNO = 0,
    TIPO_REGISTRO = 1,
    TIPO_INMEDIATO = 2,
    TIPO_MEMORIA = 3
} tipoOperando;

typedef struct
{
    int valor;
    tipoOperando Operando;
} tipoOperador;

typedef struct
{
    char opcodigo;
    tipoOperador op1, op2;

} tipoInstruccion;

uint8_t getTipoOperando(uint32_t op);
uint32_t getDireccionFisica(tipoMV programa, uint32_t direccion_logica);
void MostrarBinario(char numero);
uint32_t PropagarSigno(uint32_t valor, uint32_t cant);
uint32_t getValorCargar(tipoMV *programa, uint32_t OP, uint8_t tipo_op);
void ModificarCC(tipoMV *programa, uint32_t resultado);
void SetearAccesoMemoria(tipoMV *programa, uint32_t OP, uint8_t bytes, uint32_t direccion_fisica);
uint32_t CambiarSigno(uint32_t valor);
void ModificarIP(tipoMV *programa, char valor);


#endif // MAQUINAVIRTUAL_H_INCLUDED
