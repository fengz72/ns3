#!/bin/bash

cd ..

for i in 2 3 4 5 6 7 8 9 10
do
    ./ns3 run "dsra-main-2 --len=$i" >> dsra2.out 2>&1
done