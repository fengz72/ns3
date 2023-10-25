#!/bin/bash

cd ..

for ttl in 2 3 4 5
do
  for i in 10 20 30 40 50 60 70 80 90 100
  do
      echo "ttl = $ttl, error = $i"
      ./ns3 run "drlf-main-1 --error=$i --ttl=$ttl" >> drlf1.out 2>&1
  done
done