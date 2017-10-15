README

ftserver.c and ftclient.c

- Place ftserver.c and ftclient.c in separate directories
- Compile both using "gcc -o ftserver.c ftserver" and "gcc -o ftclient.c ftclient" respectively
- Run the server object file to start the server with usage: "ftserver [Port Number]"
- Run the client object file with usage: "ftclient <filename> <IP Address> [Port number]"
- Both the client and server will accept specific port numbers from the command line but will default to port 5103 if none is suggested.

EXTRA CREDIT:
- The ftserver is able to handle multiple threaks through forks. Copy ftclient.c into multiple directories and run them at the same time.