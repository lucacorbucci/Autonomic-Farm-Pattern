#!/bin/bash

echo "Compiling..."

make

echo "Testing..."

# name WorkerNumber TSGoal InputSize inputvalue1 inputvalue2 inputvalue3
./mainFib.out  128 1000 1000 48 46 47 > output6.txt
./mainFib.out 64 1000 1000 48 46 47 > output5.txt
./mainFib.out 32 1000 1000 48 46 47 > output4.txt
./mainFib.out 16 1000 1000 48 46 47 > output3.txt
./mainFib.out 8 1000 1000 48 46 47 > output2.txt
./mainFib.out 4 1000 1000 48 46 47 > output1.txt
./mainFib.out 2 1000 1000 48 46 47 > output0.txt
./mainFib.out 1 1000 1000 48 46 47 > output0.txt

