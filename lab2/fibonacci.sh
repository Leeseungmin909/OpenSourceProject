#!/bin/bash

echo -n  "피보나치  수열 항을 입력하시오: " 
read fibonacci

a=0
b=1
echo $a
echo $b

for ((i=2;i<fibonacci;i++)) do
	let "c = a + b"
	echo $c

	a=$b
	b=$c
done
