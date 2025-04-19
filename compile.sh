#!/bin/bash

if [ $# -ne 1 ]
then
    echo "Usage: ./compile.sh <executable_name>"
    exit
fi

gcc -Wall -o treasure_hunt treasure_hunt.c
gcc -Wall -o $1 treasure_hub.c
gcc -Wall -o monitor monitor.c
