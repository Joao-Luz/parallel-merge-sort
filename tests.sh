#/bin/bash

make > /dev/null

rm scripts/results.csv

echo -e "n,k,sort,merge,total" >> scripts/results.csv

for n in "100000" "1000000" "10000000"; do
    for k in "1" "2" "4" "8" "16" "32"; do
        echo -n "Running n=$n, k=$k... "
        for i in "1" "2" "3" "4" "5"; do
            output=$( (./parallel_merge_sort $n $k silent;) 2>&1 )

            sort=$( (echo "$output" | grep 'Sort';) 2>&1)
            sort=${sort#"Sort: "}

            merge=$( (echo "$output" | grep 'Merge';) 2>&1)
            merge=${merge#"Merge: "}

            total=$( (echo "$output" | grep 'Total';) 2>&1)
            total=${total#"Total: "}

            echo -e "$n,$k,$sort,$merge,$total" >> "scripts/results.csv"
        done
        echo -e "OK!"
    done
done