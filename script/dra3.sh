#!/bin/bash

cd ..

echo $i
./ns3 run "dra-main-3" >> dra3.out 2>&1
