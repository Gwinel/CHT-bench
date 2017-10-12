#!/bin/bash

update_rate=(0 10 20 40 80)
threads=(1 4 8 12 16 20 24 28 32 36 40 44 48 52 56 60 64)
initial_size=(1000 10000 100000 1000000 10000000)
#threads=(1 2 4 8 12 16 24 32 48 64)
#algs=(LFList LFArray LFArrayOpt SO AdaptiveArray AdaptiveArrayOpt WFList WFArray Benchmark )
#algs=(lf-ht_rcu_np hop_no_htm )
algs=(hop_htm)

for ratio in ${update_rate[*]}
do
    for alg in ${algs[*]}
    do
        for thr in ${threads[*]}
        do
		for initial in ${initial_size[*]}
        	do
           
            ofile=$alg."u$ratio"."i$initial".csv
            echo -n $thr >> ./csv/$ofile
            filename=./raw/output.$alg."n$thr"."u$ratio"."i$initial".csv
            total=0
            for i in `cat $filename`
            do
                total=$(echo $total + $i | bc)
            done
            avg=$(echo "$total / 5" | bc)
            echo -n , $avg , >> ./csv/$ofile
            cat $filename | sort -n | head -n 3 | tail -n 1 >> ./csv/$ofile
           done
        done
    done
done
