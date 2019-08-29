#!/bin/bash
# author: Luca Corbucci 
# student number: 516450


echo "Compiling..."

make

echo "Testing..."

# name WorkerNumber TSGoal InputSize inputvalue1 inputvalue2 inputvalue3
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s true > output128_12.txt
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s true > output128_22.txt
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s true > output128_32.txt
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s true > output128_42.txt
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s true > output128_52.txt

./mainFib.out 128 70 1000 42 43 41 1000 -c true -s false > output128_13.txt
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s false > output128_23.txt
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s false > output128_33.txt
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s false > output128_43.txt
./mainFib.out 128 70 1000 42 43 41 1000 -c true -s false > output128_53.txt

./mainFib.out 128 70 1000 42 43 41 1000 -f true > output128_1f.txt
./mainFib.out 128 70 1000 42 43 41 1000 -f true > output128_2f.txt
./mainFib.out 128 70 1000 42 43 41 1000 -f true > output128_3f.txt
./mainFib.out 128 70 1000 42 43 41 1000 -f true > output128_4f.txt
./mainFib.out 128 70 1000 42 43 41 1000 -f true > output128_5f.txt

# ./mainFib.out 64 70 1000 42 43 41 1000 -c false -s true > > output64_1.txt
# ./mainFib.out 64 70 1000 42 43 41 1000 -c false -s true > > output64_2.txt
# ./mainFib.out 64 70 1000 42 43 41 1000 -c false -s true > > output64_3.txt
# ./mainFib.out 64 70 1000 42 43 41 1000 -c false -s true > > output64_4.txt
# ./mainFib.out 64 70 1000 42 43 41 1000 -c false -s true > > output64_5.txt

# ./mainFib.out 32 70 1000 42 43 41 1000 -c false -s true > output32_1.txt
# ./mainFib.out 32 70 1000 42 43 41 1000 -c false -s true > output32_2.txt
# ./mainFib.out 32 70 1000 42 43 41 1000 -c false -s true > output32_3.txt
# ./mainFib.out 32 70 1000 42 43 41 1000 -c false -s true > output32_4.txt
# ./mainFib.out 32 70 1000 42 43 41 1000 -c false -s true > output32_5.txt


# ./mainFib.out 16 70 1000 42 43 41 1000 -c false -s true > output16_1.txt
# ./mainFib.out 16 70 1000 42 43 41 1000 -c false -s true > output16_2.txt
# ./mainFib.out 16 70 1000 42 43 41 1000 -c false -s true > output16_3.txt
# ./mainFib.out 16 70 1000 42 43 41 1000 -c false -s true > output16_4.txt
#./mainFib.out 16 70 1000 42 43 41 1000 -c false -s true > output16_5.txt

#./mainFib.out 8 70 1000 42 43 41 1000 -c false -s true > output8_1.txt
#./mainFib.out 8 70 1000 42 43 41 1000 -c false -s true > output8_2.txt
#./mainFib.out 8 70 1000 42 43 41 1000 -c false -s true > output8_3.txt
#./mainFib.out 8 70 1000 42 43 41 1000 -c false -s true > output8_4.txt
#./mainFib.out 8 70 1000 42 43 41 1000 -c false -s true > output8_5.txt

#./mainFib.out 4 70 1000 42 43 41 1000 -c false -s true > output4_1.txt
#-/mainFib.out 4 70 1000 42 43 41 1000 -c false -s true > output4_2.txt
#./mainFib.out 4 70 1000 42 43 41 1000 -c false -s true > output4_3.txt

#./mainFib.out 2 70 1000 42 43 41 1000 -c false -s true > output2_1.txt
#./mainFib.out 2 70 1000 42 43 41 1000 -c false -s true > output2_2.txt

#./mainFib.out 1 70 1000 42 43 41 1000 -c false -s true > output1_1.txt