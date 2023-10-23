#!/bin/bash

cd ..

for i in 2 3 4 5 6 7 8 9 10
do
    echo $i
    ./ns3 run "dra-main-2 --len=$i" >> dra2.out 2>&1
done