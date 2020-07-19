#!/bin/bash

IP=127.0.0.1
PORT=4567
CLIENT=1000
THREAD=1
MSG=100
SLEEP=100

../bin/client IP=$IP PORT=$PORT CLIENT=$CLIENT THREAD=$THREAD MSG=$MSG SLEEP=$SLEEP

read -p "Press any key to exit..." var
