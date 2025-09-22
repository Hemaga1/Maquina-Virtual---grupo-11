# Máquina Virtual - Grupo 11

Maquina virtual de Fundamentos de la Arquitectura de las Computadoras, realizada en lenguaje C.

# Integrantes
-Lara Pecora

-Maxima Maiolo

## 📂 Estructura del proyecto

```
project/
├─ source/   # Código fuente (.c y .h)
├─ obj/      # Archivos objeto generados por compilación
├─ bin/      # Ejecutable generado
└─ Makefile  # Makefile para compilar el proyecto
```

## ✅ Requisitos

- Windows  
- [MinGW](http://www.mingw.org/) con `gcc` y `mingw32-make` en el PATH
  
## ⚙️ Compilación

1. Abre **PowerShell** o **CMD** en la carpeta del proyecto.  
2. Ejecuta el siguiente comando para compilar todo:

```bash
mingw32-make
```

Esto generará:  
- Los archivos `.o` en la carpeta `obj/`  
- El ejecutable en `bin/vmx.exe`  

Si la compilación fue exitosa, verás:

```
Compilación exitosa
```

## ▶️ Ejecución

Desde la terminal, ejecuta el programa:

```bash
bin\vmx.exe "nombreDelArchivo.vmx" 
```

o

```bash
bin\vmx.exe "nombreDelArchivo.vmx" -d
```
para ejecutar ademas el Disassembler
