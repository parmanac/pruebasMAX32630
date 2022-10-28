
DEFINITION:

- Sample Code for High Level Algorithm Wrapper , ie algo suite wrapper, a single algorithm covering:
  1. HRV algorithm
  2. Respiration Rate Algorithm
  3. Stress Evaluation Algorithm
  4. Sleep Quality Algorithm
  
  Which is defined within "SSwrapper source folder" and called through " run_algorithm_wrapper_consuming_whrm_outputs()" in main method.

  and Sample Codes for SensorHub which is a prerequisite for high level algo wrapper over Authentication process.
  Which are defined within "simplest" source folder and called through demo run functions in main method.
  
  ! Authenication is needed to inituialize and run algorithms within high level algo wrapper.


BUILD steps:

- Download and setup mbed client for windows/linux from: https://github.com/ARMmbed/mbed-cli-windows-installer 
- create a project folder, from command shell browse to project folder, 
  type "mbed new projectName" 
  enter folder named with projectName.
  copy all contents of sample code source to this folder.
  include libMxmWellnessSuite.a paltform compiled static library in prokjecxt folder
  run Makefile

