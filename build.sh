#!/bin/bash

FLAG=-std=c11
LIB=-lpthread

g++ Zeus-Client/client.cpp ${FLAG} ${LIB} -o bin/client 

