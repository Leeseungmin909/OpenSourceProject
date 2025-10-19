#!/bin/bash

for ((i=1;i<=9;i++)) do
	echo "$i단" 
	for ((j=1;j<=9;j++)) do
		((c=i*j))
		echo "$i * $j = $c"
	done
done
