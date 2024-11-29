
# How to update firmware by EtherCAT

## Preparation  
Connect one or more pcbs to NIC of PC  
Power on  

## Usage  
tbox\install\bin\Release\updatefirmware.exe --firmware=string --device=unsigned short --nic=unsigned int [options] ...  
options:  
  -f, --firmware    firmware path and name (string)  
  -d, --device      id of device to update, 0 for all (unsigned short)  
  -n, --nic         id of nic to connect (unsigned int)  
  -?, --help        print this message  
  
  
## Example  
The firmware is AX58200_GpioAio.efw  
The NIC number is 3 
There are 5 EtherCAT devices, start from 1 to 5

Update the first device:  
>updatefirmware.exe -f AX58200_GpioAio.efw -d 1 -n 3  

Update all the devices:  
>updatefirmware.exe -f AX58200_GpioAio.efw -d 0 -n 3  

The update process is slow and takes a few minutes.



