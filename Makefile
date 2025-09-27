CC      = gcc
CFLAGS  = -std=c11
LDFLAGS = 

SRC_DIR = source
OBJ_DIR = obj
BIN_DIR = bin
TARGET = $(BIN_DIR)/vmx.exe

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

all: $(TARGET)
	@echo Compilacion completada con exito

# Asegura que se crean los directorios
$(TARGET): | $(BIN_DIR) $(OBJ_DIR)
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Crear directorios si no existen
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	del /Q $(OBJ_DIR)\*.o
	del /Q $(BIN_DIR)\*.exe

.PHONY: all clean  quiero que no muestre advertencias y solo un mensaje derror si no se pudo compilar