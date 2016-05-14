#!/bin/bash
echo  $1 $2
cat ResultLog.txt | grep -E $1 | grep -E $2 | sed 's/^.*FPS: //g' | sed 's/ *Data.*$//g' > calc/$1_$2
awk '{ sum += $1; count += 1 } END { print sum/count }' calc/$1_$2
