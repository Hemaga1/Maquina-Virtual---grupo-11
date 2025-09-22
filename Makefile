CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Wno-type-limits
LDFLAGS = 

SRC_DIR = source
OBJ_DIR = obj
BIN_DIR = bin
TARGET = $(BIN_DIR)/vmx.exe

# Detecta todos los archivos .c en SRC_DIR
SRC = $(wildcard $(SRC_DIR)/*.c)
# Genera la lista de objetos en OBJ_DIR
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

all: $(TARGET)

# Regla para el ejecutable
$(TARGET): $(OBJ)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR) >nul 2>&1
	@$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) >nul 2>&1
	@if %ERRORLEVEL%==0 (echo Compilacion exitosa) else (echo Compilacion fallida)

# Regla genérica para los .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR) >nul 2>&1
	@$(CC) $(CFLAGS) -c $< -o $@ >nul 2>&1

# Limpiar bin y obj
clean:
	@del /Q $(OBJ_DIR)\*.o $(TARGET) >nul 2>&1

.PHONY: all clean
