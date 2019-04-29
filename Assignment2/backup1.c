#include <mpi.h>
#include <stdio.h>
#include <math.h>
#define MASTER 0 /* task ID of master task */

int partion(int R[], int start, int end){
    //int r = select(R);
    int r = R[start];
    while(start < end){
      while( (R[end] > r) && (start < end))
        end--;
        R[start] = R[end];
      while((R[start] < r) && (start < end))
        start++;
      R[end] = R[start];
    }
    R[start] = r;
    return start;
}

void quick_sort(int R[], int start, int end){
  int r;  
  if(start < end){
      r = partion(R, start, end);
      quick_sort(R, start, r-1);
      quick_sort(R, r+1, end);
  }
}


int main(int argc, char *argv[])
{
  if (argc != 4){
    printf("Give 3 arguments: input_file_name, output_file_name, pivot_strategy_number(1-3):\n");
    return 0;
  }
  int rank, size;
  int n;
  int *data;
  //int data[4];
  double t_begin, t_end; //, time, t_total;
  char *input = argv[1];
  char *output = argv[2];
  const int pivot_strategy = atoi(argv[3]);
  int chunk;                             /* This many iterations will I do */
  int istart, istop;                  /* Variables for the local loop   */

  MPI_Init(&argc, &argv); /* Initialize MPI */

  MPI_Comm_size(MPI_COMM_WORLD, &size); /* Get the number of processors */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* Get my number                */
  // read in process 0
  t_begin = MPI_Wtime();
  if(!rank){
    FILE *f;
    f = fopen(input, "r");
    if (f == NULL){
      printf("File Error\n");
      exit(1);
    }
    fscanf(f,"%d",&n);
    //printf("%d\n", n);
    data = (int *)malloc(n * sizeof(int));
    for(int i = 0; i < n; i++){
      fscanf(f,"%d", &data[i]);
      //printf("%d\n", data[i]);
    }
    fclose(f);
  }
  // broadcast to other processes
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if(rank){
    data = (int *)malloc(n * sizeof(int));
  }
  MPI_Bcast(data, n, MPI_INT, 0, MPI_COMM_WORLD);

  chunk = n / size;   /* Number of numbers per processor */
  istart = rank * chunk;  /* Calculate start and stop indices  */
  istop = (rank + 1) * chunk - 1; /* for the local loop */
  //printf("%d, %d\n", istart, istop);
  if (rank == size - 1)
    istop = n - 1; /* Make sure the last processor      */

  quick_sort(data, istart, istop);
  //printf("qs end\n");
  MPI_Comm group_comm[8];
  int group_rank, group_size;
  int cnt = 0;
  MPI_Comm laste_comm = MPI_COMM_WORLD;
  group_rank = rank;
  group_size = size;
  while(group_size > 1){
    int color = group_rank % 2;
    MPI_split(last_comm, color, rank, &group_comm[cnt]);
    MPI_Comm_rank(group_comm, &group_rank);
    MPI_Comm_size(group_comm, &group_size);
    // naive way to choose pivot
    int p;
    if(group_rank == 0)
      p = data[(istop + istart) / 2];
    MPI_Bcast(&p, 1, MPI_INT, 0, group_comm[cnt]);
    


  }

  t_end = MPI_Wtime();
  // time = t_end - t_begin;
  // MPI_Reduce(&time, &t_total, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);

  if (rank == MASTER)
  {
    printf("%.2f", t_end - t_begin);
    // printf("%ld\t%.6f\t%.6f\n", intervals, yglobsum, t_end - t_begin);
    FILE *fp = fopen(output, "a");
    if (fp == NULL){
      printf("File Error\n");
      exit(1);
    }
    for(int i = 0; i < n; i++)
      fprintf(fp, "%d  ", data[i]);
    fclose(fp);
  }

  MPI_Finalize(); /* Shut down and clean up MPI */

  return 0;
}
