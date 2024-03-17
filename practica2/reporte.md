# Reporte de Proyecto

## Resumen

Este documento presenta el informe detallado del proyecto de desarrollo de un sistema de gestión de operaciones bancarias. El sistema permite cargar usuarios y operaciones desde archivos CSV, realizar operaciones individuales y generar reportes de carga y operaciones.

## Descripción General

El proyecto se centró en el desarrollo de un sistema de gestión bancaria para automatizar procesos relacionados con la carga de usuarios y operaciones, así como para proporcionar herramientas de seguimiento y generación de reportes.

## Contexto

En el entorno bancario actual, la eficiencia y la precisión en la gestión de operaciones son cruciales. Este proyecto surge en respuesta a la necesidad de un banco ficticio de optimizar sus procesos internos y mejorar la calidad de sus servicios.

## Objetivos

1. **Carga de Usuarios**: Desarrollar un sistema que permita cargar usuarios desde archivos CSV en múltiples hilos para mejorar la eficiencia del proceso.
2. **Carga de Operaciones**: Implementar la carga de operaciones bancarias desde archivos CSV en múltiples hilos para manejar grandes volúmenes de transacciones.
3. **Operaciones Individuales**: Habilitar funcionalidades para realizar operaciones bancarias individuales como retiros, depósitos y transferencias entre cuentas.
4. **Generación de Reportes**: Desarrollar mecanismos para generar reportes detallados de carga de usuarios y operaciones, así como reportes de errores en caso de fallos durante el proceso de carga.

## Implementación

### Carga de Usuarios

La carga de usuarios se implementó utilizando hilos en C. Se creó una función `load_users` que carga usuarios desde un archivo CSV en múltiples hilos, utilizando un mutex para garantizar la consistencia de los datos compartidos.

### Carga de Operaciones

La carga de operaciones bancarias se realizó de manera similar a la carga de usuarios, utilizando hilos para mejorar la concurrencia y la eficiencia del proceso. La función `load_operations` carga operaciones desde un archivo CSV en múltiples hilos, con un resumen detallado por hilo en el reporte de operaciones.

### Operaciones Individuales

Se implementaron funcionalidades para realizar operaciones bancarias individuales, incluyendo retiros, depósitos y transferencias. Estas operaciones se pueden realizar de forma concurrente y se reflejan correctamente en los estados de cuenta de los usuarios.

### Generación de Reportes

Se desarrollaron mecanismos para generar reportes detallados de carga de usuarios y operaciones. Los reportes incluyen información relevante sobre la cantidad de usuarios y operaciones cargadas, así como resúmenes por hilo en el caso de la carga masiva de datos. Además, se genera un reporte de errores en caso de que ocurran fallos durante el proceso de carga.

## Resultados

El proyecto logró alcanzar todos los objetivos establecidos, proporcionando una solución funcional y eficiente para la gestión de operaciones bancarias. Se mejoró la eficiencia y la precisión de los procesos de carga y se facilitó el seguimiento y la generación de reportes.

## Conclusiones

El desarrollo de este sistema de gestión bancaria demostró la importancia de la concurrencia y la eficiencia en entornos donde se manejan grandes volúmenes de datos. La implementación de hilos permitió mejorar significativamente la velocidad y el rendimiento del sistema, garantizando una experiencia fluida para los usuarios finales.

