################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adhoc/APILissandra.c \
../src/adhoc/Bitmap.c \
../src/adhoc/Consola.c \
../src/adhoc/FuncionesAPI.c \
../src/adhoc/FuncionesComp.c \
../src/adhoc/Semaforos.c 

OBJS += \
./src/adhoc/APILissandra.o \
./src/adhoc/Bitmap.o \
./src/adhoc/Consola.o \
./src/adhoc/FuncionesAPI.o \
./src/adhoc/FuncionesComp.o \
./src/adhoc/Semaforos.o 

C_DEPS += \
./src/adhoc/APILissandra.d \
./src/adhoc/Bitmap.d \
./src/adhoc/Consola.d \
./src/adhoc/FuncionesAPI.d \
./src/adhoc/FuncionesComp.d \
./src/adhoc/Semaforos.d 


# Each subdirectory must supply rules for building sources it contributes
src/adhoc/%.o: ../src/adhoc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-GameOfThreads/Bibliotecas_compartidas" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


