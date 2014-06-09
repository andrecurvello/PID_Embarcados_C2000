################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
f2802x_headers/source/F2802x_GlobalVariableDefs.obj: ../f2802x_headers/source/F2802x_GlobalVariableDefs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.2.6/bin/cl2000" -v28 -ml -mt -Ooff --include_path="C:/Users/felipeneves/Documents/TI_Workspace/PID_Embarcados_C2000/f2802x_headers/include" --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.2.6/include" -g --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="f2802x_headers/source/F2802x_GlobalVariableDefs.pp" --obj_directory="f2802x_headers/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


