################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/adhoc/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: LFS

dependents:
	-cd /home/facusalerno/workspace-tp-2019/tp-2019-1c-GameOfThreads/Bibliotecas_compartidas/Debug && $(MAKE) all

# Tool invocations
LFS: $(OBJS) $(USER_OBJS) /home/facusalerno/workspace-tp-2019/tp-2019-1c-GameOfThreads/Bibliotecas_compartidas/Debug/libBibliotecas_compartidas.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/facusalerno/workspace-tp-2019/tp-2019-1c-GameOfThreads/Bibliotecas_compartidas/Debug" -o "LFS" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) LFS
	-@echo ' '

.PHONY: all clean dependents
/home/facusalerno/workspace-tp-2019/tp-2019-1c-GameOfThreads/Bibliotecas_compartidas/Debug/libBibliotecas_compartidas.so:

-include ../makefile.targets
