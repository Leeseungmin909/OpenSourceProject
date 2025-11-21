#!/bin/bash

echo -n "별 갯수를 입력하시오: "
read starLength

for ((i = $starLength; i >= 1 ; i--)) do
	for ((j = 1; j <= i; j++)) do
		echo -n "*"
	done
	echo ""
done
