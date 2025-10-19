#!/bin/bash

count=$(ls -l | grep "^-" | wc -l)
echo $count
