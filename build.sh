#!/bin/bash

FLAG=-std=c++11
LIB=-lpthread

g++ Zeus-Client/client.cpp ${FLAG} ${LIB} -o bin/client 
g++ Zeus-Server/server.cpp ${FLAG} ${LIB} -o bin/server 

