################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ComunicacionFS.c \
../src/Consola.c \
../src/Gossiping.c \
../src/ManejoDeMemoria.c \
../src/Memoria.c \
../src/RutinasDeLiberacion.c 

OBJS += \
./src/ComunicacionFS.o \
./src/Consola.o \
./src/Gossiping.o \
./src/ManejoDeMemoria.o \
./src/Memoria.o \
./src/RutinasDeLiberacion.o 

C_DEPS += \
./src/ComunicacionFS.d \
./src/Consola.d \
./src/Gossiping.d \
./src/ManejoDeMemoria.d \
./src/Memoria.d \
./src/RutinasDeLiberacion.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-GameOfThreads/Bibliotecas_compartidas" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


