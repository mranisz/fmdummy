#!/bin/bash
make
for f in dna200; do
    for q in 1000000; do
        for m in 10 20 50; do
            for type in 1-256 1-512; do
                for s in all ACGT; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countFMDummy $type $s $f $q $m
                done
            done
        done
    done
done
for f in english200 proteins200 sources200 xml200; do
    for q in 1000000; do
        for m in 10 20 50; do
            for type in 2-256-SCBO-4 2-256-SCBO-3 2-512-SCBO-4 2-512-SCBO-3 2-256-CB-4 2-256-CB-3 2-512-CB-4 2-512-CB-3; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFMDummy $type $f $q $m
            done
        done
    done
done
for f in dna200; do
    for q in 1000000; do
        for m in 10 20 50; do
            for type in 3-512 3-1024; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFMDummy $type $f $q $m
            done
        done
    done
done
for f in dna200 english200 proteins200 sources200 xml200; do
    for q in 1000000; do
        for m in 10 20 50; do
            for type in HWT2-512 HWT2-1024 HWT4-512 HWT4-1024 HWT8-512 HWT8-1024; do
                sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                taskset -c 0 ./test/countFMDummy $type $f $q $m
            done
        done
    done
done
for lf in 0.5 0.9; do
    for f in dna200; do
        for q in 1000000; do
            for m in 6 7 8 9 10; do
                for type in 1-hash-256 1-hash-512; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countFMDummy $type ACGT 5 $lf $f $q $m
                done
            done
        done
    done
done
for lf in 0.9; do
    for f in english200 proteins200 sources200 xml200; do
        for q in 1000000; do
            for m in 6 7 8 9 10; do
                for type in 2-hash-256-SCBO-4 2-hash-256-CB-4 2-hash-256-SCBO-3 2-hash-256-CB-3 2-hash-512-SCBO-4 2-hash-512-CB-4 2-hash-512-SCBO-3 2-hash-512-CB-3; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countFMDummy $type 5 $lf $f $q $m
                done
            done
        done
    done
done
for lf in 0.5 0.9; do
    for f in dna200; do
        for q in 1000000; do
            for m in 6 7 8 9 10; do
                for type in 3-hash-512 3-hash-1024; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countFMDummy $type 5 $lf $f $q $m
                done
            done
        done
    done
done
for lf in 0.5; do
    for f in dna200; do
        for q in 1000000; do
            for m in 6 7 8 9 10; do
                for type in HWT4-hash-512 HWT4-hash-1024; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countFMDummy $type 5 $lf $f $q $m
                done
            done
        done
    done
done
for lf in 0.9; do
    for f in dna200 english200 proteins200 sources200 xml200; do
        for q in 1000000; do
            for m in 6 7 8 9 10; do
                for type in HWT4-hash-512 HWT4-hash-1024; do
                    sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"
                    taskset -c 0 ./test/countFMDummy $type 5 $lf $f $q $m
                done
            done
        done
    done
done