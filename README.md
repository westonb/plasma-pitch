# plasma-pitch

This project is a 6.78MHz (ISM band) plasma tweeter. It consists of a Class D audio modulator and a class E differential amplifier driving a resonator. 

# Hardware
Schematics/PCB files in kicad format are in the PCB folder. 

The PCB for the current hardware revision is on OSHPark: https://oshpark.com/shared_projects/XJNJnl8u

# Sofware
Compiled with and has a makefile for GCC

For the project to build you need to add the standard peripheral library and 
CMSIS library from ST to the lib folder.
