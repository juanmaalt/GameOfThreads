################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Consola.c \
../src/Gossiping.c \
../src/Kernel.c \
../src/Metrics.c \
../src/Planificador.c \
../src/Sistema_de_criterios.c \
../src/Unidad_de_ejecucion.c 

OBJS += \
./src/Consola.o \
./src/Gossiping.o \
./src/Kernel.o \
./src/Metrics.o \
./src/Planificador.o \
./src/Sistema_de_criterios.o \
./src/Unidad_de_ejecucion.o 

C_DEPS += \
./src/Consola.d \
./src/Gossiping.d \
./src/Kernel.d \
./src/Metrics.d \
./src/Planificador.d \
./src/Sistema_de_criterios.d \
./src/Unidad_de_ejecucion.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-GameOfThreads/Bibliotecas_compartidas" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


