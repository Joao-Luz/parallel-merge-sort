# Parallel Merge Sort

This is a simple implementation of the parallel 'Merge Sort' algorithm using the 'pthreads' API in C.

We expect to be able to show performance comparisons between the regular and parallel versions of the algorithm.

## Running

Simply run

    make

To build, and then

    ./parallel_merge_sort n k

To run. The program will generate a random array of size `n` and sort it using `k` threads. You can also pass `silent` as the third command line parameter to reduce the amount of output.

Alternatively, you can run

    make run

To run the program with the specified parameters defined in the `makefile`.

## Performance

No yet :(