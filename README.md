<div align="center">

# Tarea 1 — Planificador Dieciochero

Simulador de planificación concurrente de actividades mediante procesos, pipes y señales POSIX.

**Universidad Diego Portales** · Escuela de Informática y Telecomunicaciones

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![GNU Make](https://img.shields.io/badge/GNU_Make-A42E2B?style=for-the-badge&logo=gnu&logoColor=white)
![Plataforma](https://img.shields.io/badge/Plataforma-Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![Estado](https://img.shields.io/badge/Estado-Finalizado-2E8B57?style=for-the-badge)

</div>

---

## Descripción

El programa lee un archivo de actividades y construye un grafo dirigido acíclico (DAG) según sus dependencias.

Cada actividad se ejecuta mediante un proceso hijo y el planificador limita la ejecución simultánea a un máximo de `K` procesos.

Se utilizan procesos, pipes y señales POSIX. No se utilizan threads.

---

## Formato del archivo

Cada línea debe tener el formato:

```text
ID : nombre : tiempo_ms : dependencias
```

Ejemplo:

```text
1 : prender_carbon : 500 :
2 : comprar_carne : 1200 :
3 : comprar_pan : 300 :
4 : asar_longaniza : 800 : 1, 2
5 : armar_choripan : 250 : 3, 4
6 : servir_mesa : 100 : 5
```

Si el tiempo viene vacío, se asigna automáticamente un valor aleatorio entre 100 y 5000 ms.

---

## Funciones implementadas

- Parseo del archivo de entrada.
- Construcción del DAG.
- Creación de procesos mediante `fork()`.
- Control de concurrencia mediante el valor `K`.
- Comunicación entre procesos mediante pipes.
- Espera de procesos mediante `waitpid()`.
- Manejo de `SIGINT` al presionar `Ctrl+C`.
- Aislamiento de errores, abortando únicamente las ramas dependientes de una actividad fallida.
- Manejo de estados de las actividades.

---

## Estructura

```text
.
├── src
│   ├── actividad.cpp
│   ├── actividad.hpp
│   ├── main.cpp
│   ├── parser.cpp
│   ├── parser.hpp
│   ├── planificador.cpp
│   └── planificador.hpp
├── plan.txt
├── Makefile
└── README.md
```

---

## Compilación

Mediante Makefile:

```bash
make
```

También puede compilarse manualmente con:

```bash
g++ -Wall -Wextra -std=c++17 \
    src/main.cpp \
    src/parser.cpp \
    src/planificador.cpp \
    src/actividad.cpp \
    -lpthread \
    -o planificador
```

---

## Ejecución

```bash
./planificador <archivo.txt> <K>
```

Ejemplo:

```bash
./planificador plan.txt 2
```

Donde `K` corresponde a la cantidad máxima de procesos que pueden ejecutarse simultáneamente.

---

## Decisiones de diseño

- El proceso padre se encarga de coordinar la ejecución y mantener el estado del DAG.
- Las dependencias se almacenan mediante índices para acceder directamente a las actividades relacionadas.
- Se utiliza `waitpid()` para evitar espera activa.
- Los mensajes enviados mediante pipes tienen un tamaño máximo definido.
- Las actividades pueden encontrarse en estado `PENDIENTE`, `LISTA`, `CORRIENDO`, `HECHA`, `FALLIDA` o `ABORTADA`.

---

## Pruebas

Se realizaron pruebas con distintos valores de `K`, interrupción mediante `Ctrl+C`, fallos controlados y archivos de hasta 10.000 actividades.

---

## Autores

- José Peña
- Rodrigo Ruz

PD: A falta de tiempo y para mejór redacción el README se hizo con IA, gracias.
