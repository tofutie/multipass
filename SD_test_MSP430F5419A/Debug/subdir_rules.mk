################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
FAT32.obj: ../FAT32.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/home/andrew/ti/ccsv5/tools/compiler/msp430_4.1.0/bin/cl430" -vmspx --abi=eabi -g --include_path="/home/andrew/ti/ccsv5/ccs_base/msp430/include" --include_path="/home/andrew/ti/ccsv5/tools/compiler/msp430_4.1.0/include" --advice:power=all --define=__MSP430F5419A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="FAT32.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

SD_5419A.obj: ../SD_5419A.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/home/andrew/ti/ccsv5/tools/compiler/msp430_4.1.0/bin/cl430" -vmspx --abi=eabi -g --include_path="/home/andrew/ti/ccsv5/ccs_base/msp430/include" --include_path="/home/andrew/ti/ccsv5/tools/compiler/msp430_4.1.0/include" --advice:power=all --define=__MSP430F5419A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="SD_5419A.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

UART_5419A.obj: ../UART_5419A.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/home/andrew/ti/ccsv5/tools/compiler/msp430_4.1.0/bin/cl430" -vmspx --abi=eabi -g --include_path="/home/andrew/ti/ccsv5/ccs_base/msp430/include" --include_path="/home/andrew/ti/ccsv5/tools/compiler/msp430_4.1.0/include" --advice:power=all --define=__MSP430F5419A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="UART_5419A.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/home/andrew/ti/ccsv5/tools/compiler/msp430_4.1.0/bin/cl430" -vmspx --abi=eabi -g --include_path="/home/andrew/ti/ccsv5/ccs_base/msp430/include" --include_path="/home/andrew/ti/ccsv5/tools/compiler/msp430_4.1.0/include" --advice:power=all --define=__MSP430F5419A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


