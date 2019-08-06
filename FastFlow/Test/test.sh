#!/bin/bash

echo "Compiling..."

g++ -std=c++17  -g -I ../../Lib/FastFlow -O3 -finline-functions -DNDEBUG -Wall -ggdb3 -o main.out ../src/main.cpp -pthread

echo "Testing..."

# name WorkerNumber TSGoal InputSize inputvalue1 inputvalue2 inputvalue3
#./main.out 2 1000 1000 49 47 48 > output0.txt
#./main.out 4 1000 1000 49 47 48 > output1.txt
#./main.out 8 1000 1000 49 47 48 > output2.txt
#./main.out 16 1000 1000 49 47 48 > output3.txt
#./main.out 32 1000 1000 49 47 48 > output4.txt
#./main.out 64 1000 1000 49 47 48 > output5.txt
#./main.out 128 1000 1000 49 47 48 > output6.txt


