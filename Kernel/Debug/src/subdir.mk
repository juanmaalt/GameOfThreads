################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Consola.c \
../src/Kernel.c \
../src/Planificador.c \
../src/Unidad_de_ejecucion.c 

OBJS += \
./src/Consola.o \
./src/Kernel.o \
./src/Planificador.o \
./src/Unidad_de_ejecucion.o 

C_DEPS += \
./src/Consola.d \
./src/Kernel.d \
./src/Planificador.d \
./src/Unidad_de_ejecucion.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/facusalerno/workspace-tp-2019/Bibliotecas_compartidas" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


