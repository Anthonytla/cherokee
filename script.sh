#!/bin/bash

for N in {1..150}
do 
#     ./client 3490 request.txt &
#     echo $N
    curl "localhost:3490/files/index.html"
done