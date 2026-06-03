# Mini-Kernel: Simulacion de Sistema Operativo

**Proyecto Final** - Sistemas Operativos 2026-1  
Universidad Distrital Francisco Jose de Caldas

## Descripcion del Proyecto

Implementacion modular de un **Mini-Kernel** educativo en C++20 que simula los componentes principales de un sistema operativo.
La interfaz principal ahora es una ventana nativa de Windows, sin Qt y sin depender de la consola:

1. Procesos que compiten por CPU y memoria
2. E/S simultanea

La ventana muestra un estado de carga mientras simula cada escenario y usa un boton dinamico para cambiar al otro caso.

Cada escenario toma sus procesos desde los cuadros de texto:

- `CPU + memoria`: una linea por proceso con `nombre burst memoria`
- `E/S simultanea`: una linea por proceso con `nombre burst memoria ioTick`

## Requisitos

- Compilador con soporte C++20 (g++ 10+, clang 10+, MSVC 2019+)
- Windows para la interfaz grafica nativa
- Make o CMake

## Compilacion y Ejecucion

```bash
mingw32-make  # Si no se tiene instalado make
make          #     Compilar programa principal y tests
make run      # Abrir la ventana grafica del mini-kernel
make test     # Ejecutar tests unitarios
make clean    # Limpiar ejecutables y archivos temporales
```

Si prefieres CMake:

```bash
cmake -S . -B build
cmake --build build
```

## Autores

- Andres Felipe Pulido
- Victor Manuel Torres Beltran
- Janeth Oliveros Ramirez
- Juan Sebastian Rodriguez Carreno
