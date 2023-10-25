#!/bin/bash

cd ..

for ttl in 2 3 4 5
do
  for i in 2 3 4 5 6 7 8 9 10
  do
      echo "ttl = $ttl, len = $i"
      ./ns3 run "drlf-main-2 --len=$i --ttl=$ttl" >> drlf2.out 2>&1
  done
done