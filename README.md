# Analysis-of-Operating-Systems
Coursework from Analysis of Operating Systems, one of my favorite courses

pa2_char_driver is an implementation of a "device driver" where the device is really just a dynamically allocated kernel buffer. Running this device
driver/making it operational involved created a Loadable Kernel Module and a device file.






NOTE: Unfortunately this implementation was done and likely is only compatible with the Cloud VM provided to us by school 
Multi-Threading PA is an operational implementation of a multi-threaded program that reolved hostnames such as "google.com" to DNS addresses. Resolved 
hostnames are written to a txt file called resolved.txt and corresponding DNS addresses are written to a txt file called serviced.txt. Resolver threads, 
seen in multi-lookup.c, read hostnames from input files, write them to resolved.txt and place them into a shared buffer, implemented as a stack in 
array.c. Requester threads take these same hostnames out of the shared buffer/stack, look up their corresponding DNS address, and write them to 
serviced.txt.

util.c is the implementation that was given to us for DNS lookup.

This assignment was essentially a multi-threaded solution to the Bounded Buffer Problem.

To run the program from command line:
1) make clean
2) make
3) ./multi-lookup <# requester threads> <# resolver threads> <resolved.txt> <serviced.txt> [ <data files> ... ]
  for example: ./multi-lookup 2 4 resolved.txt serviced.txt input/*
 
resolved.txt and serviced.txt will be created at runtime if none exist
