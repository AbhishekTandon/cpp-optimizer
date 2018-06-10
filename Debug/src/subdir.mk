################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/InitialPointProvider.cpp \
../src/OptimizationTargets.cpp \
../src/Optimize.cpp \
../src/VectorMath.cpp \
../src/cpp-optimizer.cpp \
../src/portopt.cpp \
../src/stdafx.cpp 

OBJS += \
./src/InitialPointProvider.o \
./src/OptimizationTargets.o \
./src/Optimize.o \
./src/VectorMath.o \
./src/cpp-optimizer.o \
./src/portopt.o \
./src/stdafx.o 

CPP_DEPS += \
./src/InitialPointProvider.d \
./src/OptimizationTargets.d \
./src/Optimize.d \
./src/VectorMath.d \
./src/cpp-optimizer.d \
./src/portopt.d \
./src/stdafx.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -std=c++11 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


