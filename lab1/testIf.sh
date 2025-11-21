#!/bin/bash

echo -n  "점수를 입력하시오: "
read score

if [ $score -ge 90 ]; then
	echo "A학점" 
elif [ $score -ge 80 ]; then
	echo "B학점"
elif [ $score -ge 70 ]; then
        echo "C학점"
elif [ $score -ge 60 ]; then
        echo "D학점"
else 
	echo "F학점"
fi


