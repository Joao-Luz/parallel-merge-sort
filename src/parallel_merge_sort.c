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

int check_sorted(int* data, int n) {
    for (int i = 0; i < n-1; i++) {
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
    int n_elements = params->size;
    size_t size = sizeof(int);

    qsort(base, n_elements, size, comp);
}

void* merge(void* ptr) {
    merge_params* params = (merge_params*)ptr;

    int size_a = params->size_a;
    int size_b = params->size_b;
    int merged[size_a+size_b];

    for (int i = 0, j = 0, k = 0; k < (size_a + size_b); k++) {
        if (j >= size_b || (i < size_a && params->chunk_a[i] < params->chunk_b[j])) {
            merged[k] = params->chunk_a[i++];
        } else {
            merged[k] = params->chunk_b[j++];
        }
    }

    for (int i = 0; i < (size_a+size_b); i++) {
        params->chunk_a[i] = merged[i];
    }

}


int main(int argc, char** argv) {
    srand(time(NULL));

    if (argc < 3) {
        printf("Missing parameters!\n");
        return 0;
    }

    int n = atoi(argv[1]);
    int k = atoi(argv[2]);

    if (argc > 3 && !strcmp(argv[3], "silent")) {
        silent = 1;
    }

    int data[n];
    for (int i = 0; i < n; i++) data[i] = i;
    shuffle(data, n);

    // sorting splits
    int last_split_size = n/k + n%k;

    int size = n/k;
    pthread_t threads[k];
    sort_params params[k];

    clock_t start = clock();

    for (int i = 0; i < k; i++) {
        if (i == (k-1) && k % 2) {
            params[i] = (sort_params){&data[i*size], size + (n % k), i};
        } else {
            params[i] = (sort_params){&data[i*size], size, i};
        }
        pthread_create(threads + i, NULL, sort, (void*)(params + i));
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    // merging splits
    while (k != 1) {

        merge_params m_params[k];

        for (int i = 0; i < (k - k%2); i += 2) {
            int* chunk_a = &data[i*size];
            int* chunk_b = &data[(i+1)*size];

            int size_b = i == k-2 ? last_split_size : size;
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

    clock_t end = clock();
    float ms = 1000 * (float)(end - start) / CLOCKS_PER_SEC;

    if (check_sorted(data, n)) printf("%.0f\n", ms);
    else printf("Not Sorted :(\n");
}