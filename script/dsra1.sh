#!/bin/bash

cd ..

for i in 10 20 30 40 50 60 70 80 90 100
do
    ./ns3 run "dsra-main-1 --error=$i" >> dsra1.out 2>&1
done