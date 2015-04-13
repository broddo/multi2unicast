# multi2unicast

This small utility program was designed to connect to a multicast address and port and relay any packets received on this connection to predefined UDP port. I originally wrote this program as Matlab does not support the ability to connect to muticast network sources (without a lot of difficulty anyway). Matlab does work very well with UDP network sources but the external instrument that I needed to extract data from only operated via multicast. This program operated as a "go-between" for the two, but, of course, could be used in an senario. 

This program was written on a raspberry pi but there is no reason why it shouldn't work on other platforms provided that the target machine's network adaptor/kernel support multicast. 

To compile, simply:
gcc multi2uni.c -o multi2uni

To execute:
./multi2uni <multicast address> <multicast port>

The UDP address and port can be adjusted as desired in multi2uni.c
