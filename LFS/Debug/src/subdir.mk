################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/APILissandra.c \
../src/Consola.c \
../src/Lissandra.c 

OBJS += \
./src/APILissandra.o \
./src/Consola.o \
./src/Lissandra.o 

C_DEPS += \
./src/APILissandra.d \
./src/Consola.d \
./src/Lissandra.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/facusalerno/workspace-tp-2019/Bibliotecas_compartidas" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


