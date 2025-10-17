#ifndef MAQUINAVIRTUAL_H_INCLUDED
#define MAQUINAVIRTUAL_H_INCLUDED


#include <stdint.h>
// #define TAMANIO_MEMORIA 16384
#define NUM_REGISTROS 32

//#define VERSION 1
#define IDENTIFICADOR "VMX25"
#define IDENTIFICADOR_VMI "VMI25"

typedef enum
{
    // Acceso a memoria
    LAR = 0,
    MAR = 1,
    MBR = 2,

    // Instruccion
    IP = 3,
    OPC = 4,
    OP1 = 5,
    OP2 = 6,

    // Pila
    SP = 7,
    BP = 8,

    // Registros de proposito general
    EAX = 10,
    EBX = 11,
    ECX = 12,
    EDX = 13,
    EEX = 14,
    EFX = 15,

    // Acumulador y codigo de condicion
    AC = 16,
    CC = 17,

    // Segmentos
    CS = 26,
    DS = 27,
    ES = 28,
    SS = 29,
    KS = 30,
    PS = 31

} t_registro;

typedef struct MV
{
    uint8_t *memoria;
    uint32_t registros[NUM_REGISTROS];
    uint16_t TS[8][2];
    uint16_t tamanioMemoria;
    char *nombreVMX;
    char version;
} tipoMV;



typedef struct Tparametros
{
    char *vmxfile;
    char *vmifile;
    int disassembler;
    int tamanioMemoria;
    int argc;
    char **constantes;
} Tparametros;


typedef enum
{
    NINGUNO = 0,
    TRegistro = 1,
    TInmediato = 2,
    TMemoria = 3
} tipoOperando;

extern const char *NombreRegistro[32];
int leerEncabezado(const char *filename, tipoMV *mv);
void Disassembler(tipoMV programa);
void PrintOperando(uint32_t op);
void InicializarRegistros(uint32_t registros[]);
void ejecutar_maquina(tipoMV *mv);
void leerParametros(int argc, char *argv[], Tparametros *parametros);
void crearParamSegment(tipoMV *mv, Tparametros *parametros);
int leerVMX(const char *filename, tipoMV *mv);
void iniciarTablaSegmentos(tipoMV *mv, uint16_t sizes[], unsigned short int cantSegments);

uint8_t getTipoOperando(uint32_t op);
uint32_t getDireccionFisica(tipoMV programa, uint32_t direccion_logica);
void MostrarBinario(uint32_t numero);
uint32_t StringABinario(char cadena[33]);
uint32_t PropagarSigno(uint32_t valor, uint32_t cant);
uint32_t getValorCargar(tipoMV *programa, uint32_t OP);
void setOperando(tipoMV *programa, uint32_t OP, uint32_t valor_cargar);
void ModificarCC(tipoMV *programa, uint32_t resultado);
void SetearAccesoMemoria(tipoMV *programa, uint32_t OP, uint8_t bytes, uint32_t direccion_fisica);
uint32_t CambiarSigno(uint32_t valor);
void ModificarIP(tipoMV *programa, uint32_t valor);
void pushearValor(tipoMV *programa, uint32_t valor);


#endif // MAQUINAVIRTUAL_H_INCLUDED
