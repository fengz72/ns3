#!/bin/bash

cd ..

for i in 2 4 6 8 10 12 14 16 18 20
do
    echo $i
    ./ns3 run "dra-main-1 --error=$i" >> dra1.out 2>&1
done

for i in 30 40 50 60 70 80 90 100
do
    echo $i
    ./ns3 run "dra-main-1 --error=$i" >> dra1.out 2>&1
done