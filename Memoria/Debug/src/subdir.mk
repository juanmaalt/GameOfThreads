################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/API/APIMemoria.c \
../src/API/UtilitariasAPI.c 

OBJS += \
./src/API/APIMemoria.o \
./src/API/UtilitariasAPI.o 

C_DEPS += \
./src/API/APIMemoria.d \
./src/API/UtilitariasAPI.d 


# Each subdirectory must supply rules for building sources it contributes
src/API/%.o: ../src/API/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-GameOfThreads/Bibliotecas_compartidas" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


