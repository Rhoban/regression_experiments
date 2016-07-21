#!/bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $0 <folders>"
    exit -1
fi

i=0
for arg in $@
do
    regPaths[$i]="${arg}/benchmark_regression.csv"
    maxPaths[$i]="${arg}/benchmark_max.csv"
    ((i++))
done

Rscript plotBenchmark.r ${regPaths[@]}
Rscript plotBenchmarkMax.r ${maxPaths[@]}
