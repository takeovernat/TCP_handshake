Natnael Teshome
notice the server only runs on UNT's cse02 server, if you want it to run on your own server you have to modify the server ip address in the client.

to run server using makefile on cse02
make serv
compiles and runs server with chosen port 7777

to run client using make file 
make cli
compiles and runs client with chosen port 7777

to remove
make clean
removes all log files and binaries

uses TCP connection and port 7777 to make a 3 way handshake connection and disconnetion
using TCP segments, by evaluating checksums and flag bits indicatded in the tcp segmenst.
