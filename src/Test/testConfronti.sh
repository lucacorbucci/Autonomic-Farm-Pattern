echo "Testing...."

# Test MainFib -> task pesante
./mainFib.out 128 15 1000 41 43 42 1500 -s true
./mainFib.out 128 15 1000 39 41 40 500 -s true
./mainFib.out 128 15 1000 39 41 40 500 -s true
./mainFib.out 128 15 1000 39 41 40 500 -s true
./mainFib.out 128 15 1000 39 41 40 500 -s true

./mainFib.out 128 15 1000 41 43 42 1500 -s false
./mainFib.out 128 15 1000 39 41 40 500 -s false
./mainFib.out 128 15 1000 39 41 40 500 -s false
./mainFib.out 128 15 1000 39 41 40 500 -s false
./mainFib.out 128 15 1000 39 41 40 500 -s false

# Test MainFib -> task leggero
./mainFib.out 4 1 1000000 7 4 5 2000 -s true
./mainFib.out 4 1 1000000 7 4 5 2000 -s false

# Test MainPrime -> task leggero
./mainPrime.out 4 1 10000000 2147487 2147483 214745 2000 -s true
./mainPrime.out 4 1 10000000 2147487 2147483 214745 2000 -s false