#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define min(a, b) a < b ? a : b;

int silent = 0;

typedef struct {
    int* base;
    int size;
    int id;
} sort_params;

typedef struct {
    int size_a, size_b;
    int* chunk_a;
    int* chunk_b;
} merge_params;

int check_sorted(int* data, size_t n) {
    for (size_t i = 0; i < n-1; i++) {
        if (!(data[i+1] > data[i])) return 0;
    }

    return 1;
}

void shuffle(int* data, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = data[j];
          data[j] = data[i];
          data[i] = t;
        }
    }
}

int comp(const void * a, const void * b) 
{
    int f = *((int*)a);
    int s = *((int*)b);
    return f - s;
}

void* sort(void* ptr) {
    sort_params* params = (sort_params*)ptr;

    if (!silent)
        printf("Thread %d: Sorting %d elements.\n", params->id, params->size);

    int* base = params->base;
    size_t n_elements = params->size;
    size_t size = sizeof(int);

    qsort(base, n_elements, size, comp);
}

void* merge(void* ptr) {
    merge_params* params = (merge_params*)ptr;

    size_t size_a = params->size_a;
    size_t size_b = params->size_b;
    int* merged = malloc((size_a+size_b)*sizeof(int));

    for (size_t i = 0, j = 0, k = 0; k < (size_a + size_b); k++) {
        if (j >= size_b || (i < size_a && params->chunk_a[i] < params->chunk_b[j])) {
            merged[k] = params->chunk_a[i++];
        } else {
            merged[k] = params->chunk_b[j++];
        }
    }

    for (size_t i = 0; i < (size_a+size_b); i++) {
        params->chunk_a[i] = merged[i];
    }

}


int main(int argc, char** argv) {
    srand(time(NULL));

    if (argc < 3) {
        printf("Missing parameters!\n");
        return 0;
    }

    size_t n = atoi(argv[1]);
    int k = atoi(argv[2]);

    if (argc > 3 && !strcmp(argv[3], "silent")) {
        silent = 1;
    }

    int* data = malloc(n*sizeof(int));
    for (size_t i = 0; i < n; i++) data[i] = i;
    shuffle(data, n);

    // sorting splits
    size_t last_split_size = n/k + n%k;

    size_t size = n/k;
    pthread_t threads[k];
    sort_params params[k];

    struct timespec sort_start, sort_finish;
    double sort_elapsed;

    clock_gettime(CLOCK_MONOTONIC, &sort_start);

    for (int i = 0; i < k; i++) {
        if (i == (k-1) && n % k) {
            params[i] = (sort_params){&data[i*size], size + (n % k), i};
        } else {
            params[i] = (sort_params){&data[i*size], size, i};
        }
        pthread_create(threads+i, NULL, sort, (void*)(params + i));
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &sort_finish);

    sort_elapsed = (sort_finish.tv_sec - sort_start.tv_sec);
    sort_elapsed += (sort_finish.tv_nsec - sort_start.tv_nsec) / 1000000000.0;

    // Merge

    struct timespec merge_start, merge_finish;
    double merge_elapsed;

    clock_gettime(CLOCK_MONOTONIC, &merge_start);
    while (k != 1) {

        merge_params m_params[k];

        for (int i = 0; i < (k - k%2); i += 2) {
            int* chunk_a = &data[i*size];
            int* chunk_b = &data[(i+1)*size];

            size_t size_b = i == k-2 ? last_split_size : size;
            m_params[i] = (merge_params){ size, size_b, chunk_a, chunk_b };

            if (i == k-2) {
                last_split_size += size;
            }

            pthread_create(threads+i, NULL, merge, (void*)(m_params+i));
        }

        for (int i = 0; i < (k - k%2); i += 2) {
            pthread_join(threads[i], NULL);
        }

        size *= 2;
        k = k/2 + k%2;

    }

    clock_gettime(CLOCK_MONOTONIC, &merge_finish);

    merge_elapsed = (merge_finish.tv_sec - merge_start.tv_sec);
    merge_elapsed += (merge_finish.tv_nsec - merge_start.tv_nsec) / 1000000000.0;

    if (check_sorted(data, n)) {
        printf("Sort: %.4f\n", sort_elapsed);
        printf("Merge: %.4f\n", merge_elapsed);
        printf("Total: %.4f\n", sort_elapsed + merge_elapsed);
    }
    else printf("Not Sorted :(\n");
}