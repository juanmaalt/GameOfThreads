################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../parser/compresor_describe.c \
../parser/compresor_direccion.c \
../parser/parser_comando.c 

OBJS += \
./parser/compresor_describe.o \
./parser/compresor_direccion.o \
./parser/parser_comando.o 

C_DEPS += \
./parser/compresor_describe.d \
./parser/compresor_direccion.d \
./parser/parser_comando.d 


# Each subdirectory must supply rules for building sources it contributes
parser/%.o: ../parser/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


