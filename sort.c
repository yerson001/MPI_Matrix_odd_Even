#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* the number of data elements in each process */
#define N 150000

int isSorted;

void secuencial(int arr[], int n)
{
    isSorted = 0; // Initially array is unsorted
  
    while (!isSorted) {
        isSorted = 1;
  
        // Perform Bubble sort on odd indexed element
        for (int i = 1; i <= n - 2; i = i + 2) {
            if (arr[i] > arr[i + 1]) {
              int temp;
              temp = arr[i];
              arr[i] = arr[i + 1];
              arr[i+1] = temp;
                isSorted = 0;
            }
        }
  
        // Perform Bubble sort on even indexed element
        for (int i = 0; i <= n - 2; i = i + 2) {
            if (arr[i] > arr[i + 1]) {
               int temp;
              temp = arr[i];
              arr[i] = arr[i + 1];
              arr[i+1] = temp;
                isSorted = 0;
            }
        }
    }
  
    return;
}

/* initialize the data to random values based on rank (so they're different) */
void init(int* data, int rank) {
  int i;
  srand(rank);
  for (i = 0; i < N; i++) {
    data[i] = rand( ) % 100;
  }
}

/* print the data to the screen */
void print(int* data, int rank) {
  int i;
  printf("Process %d: ");
  for (i = 0; i < N; i++) {
    printf("%d ", data[i]);
  }
  printf("\n");
}

/* comparison function for qsort */
int cmp(const void* ap, const void* bp) {
  int a = * ((const int*) ap);
  int b = * ((const int*) bp);

  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  } else {
    return 0;
  }
}

/* find the index of the largest item in an array */
int max_index(int* data) {
  int i, max = data[0], maxi = 0;

  for (i = 1; i < N; i++) {
    if (data[i] > max) {
      max = data[i];
      maxi = i;
    }
  }
  return maxi;
}

/* find the index of the smallest item in an array */
int min_index(int* data) {
  int i, min = data[0], mini = 0;

  for (i = 1; i < N; i++) {
    if (data[i] < min) {
      min = data[i];
      mini = i;
    }
  }
  return mini;
}


/* do the parallel odd/even sort */
void parallel_sort(int* data, int rank, int size) {
  int i;
  int other[N];
  for (i = 0; i < size; i++) {
    qsort(data, N, sizeof(int), &cmp);
    int partener;

    if (i % 2 == 0) {
      if (rank % 2 == 0) {
        partener = rank + 1;
      } else {
        partener = rank - 1;
      }
    } else {
      if (rank % 2 == 0) {
        partener = rank - 1;
      } else {
        partener = rank + 1;
      }
    }
    if (partener < 0 || partener >= size) {
      continue;
    }
    if (rank % 2 == 0) {
      MPI_Send(data, N, MPI_INT, partener, 0, MPI_COMM_WORLD);
      MPI_Recv(other, N, MPI_INT, partener, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
      MPI_Recv(other, N, MPI_INT, partener, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send(data, N, MPI_INT, partener, 0, MPI_COMM_WORLD);
    }
    if (rank < partener) {
      while (1) {
        int mini = min_index(other);
        int maxi = max_index(data);
        if (other[mini] < data[maxi]) {
          int temp = other[mini];
          other[mini] = data[maxi];
          data[maxi] = temp;
        } else {
          break;
        }
      }
    } else {
      while (1) {
        int maxi = max_index(other);
        int mini = min_index(data);
        if (other[maxi] > data[mini]) {
          int temp = other[maxi];
          other[maxi] = data[mini];
          data[mini] = temp;
        } else {
          /* else stop because the largest are now in data */
          break;
        }
      }
    }
  }
}
int main() {
  /* our rank and size */
  int rank, size;

  double start,finish;
  double start1,finish1;
  /* our processes data */
  int data[N];

  /* initialize MPI */
  MPI_Init(NULL,NULL);

  /* get the rank (process id) and size (number of processes) */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /* initialize the data */
  init(data, rank);

  start = MPI_Wtime();
  parallel_sort(data, rank, size);
  finish = MPI_Wtime();
//   print(data, rank);
  printf(" Elapsed time = %e seconds\n", finish-start);



  start1 = MPI_Wtime();
  secuencial(data, size);
  finish1 = MPI_Wtime();
  printf("SECUENCIAL = %e seconds\n", finish1-start1);
  /* quit MPI */
  MPI_Finalize( );
  return 0;
}
