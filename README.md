
## 🎯 Propósito y Funcionalidad

**Proyecto**: Sistema de Plugins Dinámicos para GNU objdump

**Objetivo Principal**: Extender objdump (herramienta de desensamblado de GNU Binutils) con un sistema de callbacks/hooks que permita analizar instrucciones en tiempo real durante el desensamblado, sin modificar sustancialmente el código.

**Construccion:**
```
git clone git://sourceware.org/git/binutils-gdb.git
```
Luego:
```cd ~/Desktop/plugindecero/binutils-gdb
rm -rf build
mkdir build
cd build
../configure \
  --disable-gdb \
  --disable-gdbserver \
  --disable-gas \
  --disable-ld \
  --disable-nls
make all-binutils
 ```
**Funcionalidad Implementada**:
- Sistema de plugins dinámicos (.so) cargados en runtime vía dlopen/dlsym
- 5 tipos de hooks/callbacks:
  1. **Instrucción válida**: Analiza cada instrucción desensamblada
  2. **Instrucción inválida**: Captura errores de desensamblado
  3. **Archivo**: Notifica cuando se procesa un nuevo archivo
  4. **Sección**: Notifica cambios de sección (.text, .data, etc.)
  5. **Etiqueta/Símbolo**: Notifica cuando se encuentra un símbolo

---

## 🛠️ Enfoque y Metodología

### **Restricciones del Proyecto**
- **Minimizar cambios en opcodes**: Solo agregar llamadas a hooks, sin alterar lógica de desensamblado
- **Arquitectura modular**: Plugin como biblioteca compartida independiente
- **Compatibilidad**: No romper funcionalidad existente de objdump


### **Fases de Desarrollo**

1. **Diseño de interfaz** (xxxx_dis_info.h, objdump_plugin.h)
   - Definir struct instr_info con campos necesarios dependiendo la arquitectura
   - Tipificar callbacks para plugin

2. **Integración en desensamblador**
   - Captura de mnemónicos y operandos ANTES del printf
   - Inserción de llamadas a hooks en momentos clave
   - Manejo de instrucciones inválidas

3. **Carga dinámica** (objdump.c)
   - Sistema de carga de plugin con dlopen
   - Funciones puente entre desensamblador y plugin
   - Integración de hooks en flujo de objdump

4. **Implementación del plugin** (objdump_plugin.c)
   - Análisis sintáctico de operandos AT&T
   - Clasificación por tipo (Reg/Mem/Imm/Addr)
   - Contador de instrucciones y estadísticas

---

## 📊 Resultados Obtenidos

### **Salida Ejemplo**
```
[Instruction #287] or  [OPERAND x2] %al (%rax)
[BAD] bad instruction encountered
[Instruction #288] add  [OPERAND x4] %bl (%rax %rax 1)
[Instruction #289] add  [OPERAND x2] %al (%rax)
[Instruction #290] cmp  [OPERAND x2] $0x0 %al
[Instruction #291] add  [OPERAND x2] %al (%rax)
[Instruction #292] add  [OPERAND x2] %al (%rax)
[Instruction #293] add  [OPERAND x2] %al (%rax)
[Instruction #294] or  [OPERAND x2] (%rax) %eax
[Instruction #295] add  [OPERAND x2] %al (%rax)
[Instruction #296] add  [OPERAND x2] %al 0xe(%rbp)
[Instruction #297] adc  [OPERAND x2] %al 0x60d4302(%rsi)
[Instruction #298] or  [OPERAND x2] $0x7 %al
[Instruction #299] or  [OPERAND x2] %al (%rax)

[INFO] Instructions written to: disasm_output.txt

╔════════════════════════════════════════════════════════╗
[INFO] Plugin closed                                     ║
║               INSTRUCTION STATISTICS                   ║
║ Architecture: elf64-x86-64                             ║
╠════════════════════════════════════════════════════════╣
║ Total files processed:                 3               ║
║ Total sections processed:              15              ║
║ Total labels processed:                10              ║
║ Total instructions:                    299             ║
║ Invalid instructions (BAD):            19              ║
╠════════════════════════════════════════════════════════╣
║ Calls (CALL):                          0               ║
║ Jumps (JUMP):                          16              ║
║ Returns (RET):                         6               ║
║ Instructions with REX.W:               21              ║
║ Atomic operations (LOCK):              0               ║
║ String operations (REP):               6               ║
║ Stack canaries detected:               0               ║
╚════════════════════════════════════════════════════════╝
║ Regex pattern dereference (file):      201             ║
║ Regex pattern indirect call (file):    0               ║
╚════════════════════════════════════════════════════════╝
```

### **Archivos**
- **NUEVOS** (3):
    
/plugin
  - objdump_plugin.h 
  - objdumo_plugin_arm.
  - objdump_plugin_mips.g
  - objdump_plugin_x86_64.h
  - objdump_plugin_i386.h
  - objdump_plugin.cpp - plugin con ejemplos de uso
  - objdump_pluginv2.cpp -plugin basico

/opcodes
  - i386_dis_instr_info.h - Definición de struct compartida
  - arm_dis_instr_info.h
  - x86_64_dis_instr_info.h
  - mips_dis_instr_info.h


 **MODIFICADOS** :

/opcodes
  - i386-dis.c 
  - arm-dis.c
  - mips-dis.c

/binutils
  - objdump.c - Sistema de carga dinámica 
---
## **RESUMEN ARQUITECTURA objdump.c**
```
main() inicio
    ↓
┌─────────────────────┐
│ init_plugin()       │ ← AGREGADO
│   dlopen(.so)       │
│   dlsym(callbacks)  │
└─────────────────────┘
    ↓
Procesar archivos...
    ↓
┌─────────────────────┐
│ file_hook()         │ ← AGREGADO 
└─────────────────────┘
    ↓
┌─────────────────────┐
│ section_hook()      │ ← AGREGADO 
└─────────────────────┘
    ↓
┌─────────────────────┐
│ label_hook()        │ ← AGREGADO 
└─────────────────────┘
    ↓
Desensamblar...
    ↓
arquitectura-dis.c llama →  instruccion_hook() ← AGREGADO
                         ↓
                    g_plugin_callback(ins)
                         ↓
                    objdump_plugin.so
    ↓
main() final
    ↓
┌─────────────────────┐
│ cleanup_plugin()    │ ← AGREGADO
│   print_stats()     │
│   dlclose(.so)      │
└─────────────────────┘
```
---


## **RESUMEN DE LA ARQUITECTURA arquitecturaX-dis.c**

```
┌─────────────────────────────────────────┐
│  i386-dis.c (opcodes)                   │
│  ┌──────────────────────────────┐       │
│  │ print_insn()                 │       │
│  │  1. Decodifica instrucción   │       │
│  │  2. Llena obuf con mnemonic  │       │
│  │  3. ★ GUARDA a ins.mnemonic  │◄──────┼─── CAMBIO 3: Captura ANTES de imprimir
│  │  4. Imprime mnemonic         │       │
│  │  5. Construye op_txt[]       │       │
│  │  6. ★ GUARDA a ins.operands  │◄──────┼─── CAMBIO 4: Captura ANTES de perder scope
│  │  7. Imprime operandos        │       │
│  │  8. ★ instruccion_hook(&ins) │───────┼─── CAMBIO 5: Notifica al plugin
│  │  9. return                   │       │
│  └──────────────────────────────┘       │
└─────────────────────────────────────────┘
           ↓ (pasa struct completo)
┌─────────────────────────────────────────┐
│  objdump.c (binutils)                   │
│  ┌──────────────────────────────┐       │
│  │ instruccion_hook(ins)        │       │
│  │   → g_plugin_callback(ins)   │───────┼─── Llama al plugin .so
│  └──────────────────────────────┘       │
└─────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────┐
│  objdump_plugin.so                      │
│  ┌──────────────────────────────┐       │
│  │ plugin_callback(ins)         │       │
│  │   → Lee ins->mnemonic        │       │
│  │   → Lee ins->operands        │       │
│  │   → Analiza y muestra        │       │
│  └──────────────────────────────┘       │
└─────────────────────────────────────────┘
```

**arquitecturaX_dis_info.h es NUEVO** - fue creado específicamente para el sistema de plugins. 

**Originalmente** en binutils-gdb:
- `struct instr_info` estaba definida DENTRO de arquitecturaX-dis.c (era privada)
- No había ningún header compartido para esta estructura
- Solo opcodes podía usar esta estructura

**El problema**:
- El plugin (en binutils) necesita acceder a `struct instr_info`
- Pero la definición estaba encerrada en arquitecturaX-dis.c
- No se puede incluir un archivo .c desde otro módulo

**La solución**:
1. **Extraer** la definición de `struct instr_info` de arquitecturaX-dis.c
2. **Crear** nuevo header en especifico para cierta arquitectura_dis_info.h  con campos necesarios de cada arquitectura. La definicion de struc_instr_info en .c moverla a .h
3. **Compartir** entre opcodes/ y binutils/

---

## **RESUMEN DE CAMBIOS EN CASO DE i386_dis_info.h**

| Elemento | Estado | Explicación |
|----------|--------|-------------|
| **Archivo completo** | NUEVO | No existía, se creó para compartir definiciones |
| `MAX_OPERANDS` | Movido | Estaba en i386-dis.c, se extrajo aquí |
| `MAX_OPERAND_BUFFER_SIZE` | Movido | Estaba en i386-dis.c, se extrajo aquí |
| `MAX_CODE_LENGTH` | Movido | Estaba en i386-dis.c, se extrajo aquí |
| `enum address_mode` | Movido | Estaba en i386-dis.c, se extrajo aquí |
| `enum x86_64_isa` | Movido | Estaba en i386-dis.c, se extrajo aquí |
| `enum evex_type` | Movido | Estaba en i386-dis.c, se extrajo aquí |
| `struct instr_info` (campos 1-50) | Movido | Estaba en i386-dis.c, se extrajo aquí |
| `char mnemonic[64]` | **NUEVO** | Agregado al struct para guardar mnemónico |
| `char operands[256]` | **NUEVO** | Agregado al struct para guardar operandos |

---

## **¿POR QUÉ ERA NECESARIO CREAR ESTE ARCHIVO?**

**Problema de arquitectura**:
```
opcodes/ ──┐
           ├─── NO pueden compartir código directamente
binutils/──┘     (módulos separados)
```

**Solución con header compartido**:
```
                i386_dis_info.h
                       ↑
         ┌─────────────┼─────────────┐
         ↓                           ↓
    opcodes/i386-dis.c      binutils/objdump.c
    (productor de datos)    (consumidor de datos)
```

---

# **objdump_plugin.h y objdump_plugin.c**
---

Estos **2 archivos son COMPLETAMENTE NUEVOS** - no existían en binutils-gdb original. Se crearon específicamente para implementar el sistema de plugins.

---

## **ARCHIVO 1: objdump_plugin.h (Header del plugin)**

Este archivo **DEFINE LA INTERFAZ** entre objdump y el plugin.

```c
#ifndef OBJDUMP_PLUGIN_H
#define OBJDUMP_PLUGIN_H
// Generic callback typedefs 
typedef void (*plugin_cb_t)(void *); 
typedef void (*bad_plugin_cb_t)(const char *);
typedef void (*file_plugin_cb_t)(const char *);
typedef void (*section_plugin_cb_t)(const char *, const char *);
typedef void (*label_plugin_cb_t)(const char *, const char *, unsigned long);
#endif // OBJDUMP_PLUGIN_H
```
objdump.c necesita saber:
1. Qué funciones buscar en el plugin .so
2. Qué tipo de parámetros aceptan esas funciones

**Solución**: Definir **tipos de función** (function pointers) para cada callback

---

## **FLUJO COMPLETO DE COMUNICACIÓN**

```
┌────────────────────────────────────────────────────┐
│  i386-dis.c (opcodes)                              │
│  - Decodifica instrucción                          │
│  - Llena ins.mnemonic = "push"                     │
│  - Llena ins.operands = "%rbp"                     │
│  - Llama: instruccion_hook(&ins)                   │
└────────────────┬───────────────────────────────────┘
                 │
                 ↓
┌────────────────────────────────────────────────────┐
│  objdump.c (binutils)                              │
│  void instruccion_hook(struct instr_info *ins) {   │
│      g_plugin_callback(ins);  ← llama al .so       │
│  }                                                  │
└────────────────┬───────────────────────────────────┘
                 │
                 ↓
┌────────────────────────────────────────────────────┐
│  objdump_plugin.so (compilado desde .c)            │
│                                                     │
│  void plugin_callback(struct instr_info *ins) {    │
│      insn_count++;                                 │
│      printf("[Instrucción #%lu] ", insn_count);    │
│      printf("%-8s", ins->mnemonic);  ← "push"      │
│                                                     │
│      // Separar operandos por coma                 │
│      tokens = split(ins->operands, ",");           │
│                                                     │
│      // Analizar cada operando                     │
│      explain_operand("%rbp");                      │
│          ↓                                          │
│      [Reg: %rbp]  ← imprime explicación            │
│  }                                                  │
└────────────────────────────────────────────────────┘
```
---
## 🏗️ Soporte Multiarquitectura y Ejecución de Tests

### Implementación Multiarquitectura

El sistema de plugins está diseñado para soportar múltiples arquitecturas (ARM, MIPS, x86_64, i386) mediante el uso de macros de preprocesador (`-DTARGET_X86_64`, `-DTARGET_ARM`, etc.) al compilar el plugin. Esto permite que el mismo código fuente del plugin adapte su comportamiento y estructuras de datos según la arquitectura objetivo.

Cada arquitectura tiene su propio header de definición de instrucciones (por ejemplo, `arm_dis_instr_info.h`, `mips_dis_instr_info.h`, etc.), y el plugin selecciona la estructura adecuada en tiempo de compilación.

### Compilación del Plugin para Cada Arquitectura

Para compilar el plugin para una arquitectura específica, utiliza el flag correspondiente:

**Para ARM:**
```sh
g++ -shared -fPIC -DTARGET_ARM -o objdump_plugin.so ../plugins/objdump_plugin.cpp
./objdump -D libtest_arm.a
```
**Para MIPS:**
```sh
g++ -shared -fPIC -DTARGET_MIPS -o objdump_plugin.so ../plugins/objdump_plugin.cpp
./objdump -D libtest_mips.a
```
**Para x86_64:**
```sh
g++ -shared -fPIC -DTARGET_X86_64 -o objdump_plugin.so ../plugins/objdump_plugin.cpp
./objdump -D libtest_x86_64.a
```
**Para i386:**
```sh
g++ -shared -fPIC -DTARGET_I386 -o objdump_plugin.so ../plugins/objdump_plugin.cpp
./objdump -D Test.a
```

### Ejecución de Tests

En la carpeta `tests/` se encuentra un script automatizado para validar el funcionamiento del sistema de plugins en las distintas arquitecturas. El script ejecuta objdump con el plugin sobre archivos de prueba y compara la salida con resultados esperados.

Para correr todos los tests:
```sh
python3 tests/run_tests.py
```
**Test:**
- Compila el plugin para cada arquitectura
- Ejecuta objdump sobre los archivos de prueba correspondientes
- Verifica que la salida del plugin sea la esperada (mnemónicos, operandos, estadísticas, etc.)
- Reporta el resultado de cada test (OK/FAIL)

Esto asegura que el sistema de plugins funcione correctamente y de forma consistente en todas las arquitecturas soportadas.

---
## 🚀 Plugin vs JASM

La siguiente tabla muestra una comparación de tiempos de desensamblado entre el sistema de plugins y JASM, usando binarios de distintos tamaños. Se observa que el plugin ofrece una mejora significativa en velocidad, especialmente en binarios pequeños y medianos, y mantiene una ventaja clara incluso en archivos grandes.

| Tamaño del Binario (KB) | Tiempo Plugin (s) | Tiempo JASM (s) | Ganancia (Speedup) |
|------------------------:|------------------:|----------------:|:------------------:|
| 15                      | 0.04              | 3.19            | ~79.7x             |
| 39                      | 0.15              | 2.1             | 14.0x              |
| 139                     | 0.48              | 2.6             | 5.4x               |
| 7,700                   | 15.91             | 34              | 2.1x               |
| 12,000                  | 30.86             | 77.5            | 2.5x               |
| 91,000                  | 242.27            | 571.6           | 2.4x               |

**Interpretación:**

- Para binarios pequeños y medianos, el plugin es hasta 80 veces más rápido que JASM.
- En binarios grandes, la mejora se mantiene entre 2x y 2.5x, lo que sigue representando un ahorro de tiempo considerable.
- El sistema de plugins aprovecha la integración directa con objdump y el procesamiento eficiente en C/C++, evitando sobrecostos de interpretación o frameworks externos.

---
## Forma de uso:

### compilacion
Elegir arquitetura para analizar, seleccionar el target conveniente.
```sh
g++ -shared -fPIC -DTARGET_I386 -o objdump_plugin.so ../plugins/objdump_plugin.cpp
./objdump -D Test.a
```
***Nota:*** tener en cuenta que el binario creado del plugin debe estar en el mismo directorio que el binario de objdump.


---

# Sin plugin
./objdump -d binario

# Con plugin
./objdump -d --plugin objdump_plugin.so binario