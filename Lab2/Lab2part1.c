/*
 * Evan Chou 
 * Lab 2 Part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 3
#define M 3
#define L 3

double matrixA[N][M];
double matrixB[M][L];
double matrixC[N][L];

struct set
{
	int number;
};

pthread_t matrix_thread[N];

//only creates j*k and assigns value for matrixC
void *thread(void *rst)
{
    int j,k;
    double temp=0;
    struct set *data=rst;

    for(j=0;j<L;j++)
    {
        for(k=0;k<M;k++)
        {

            temp+=matrixA[data->number][k] * matrixB[k][j];
        }
        matrixC[data->number][j]=temp;
    }
    pthread_exit(0);
}

void printMatrix(int r, int c, double matrix[r][c])
{
    int i, j;
    for(i=0;i<r;i++)
    {
        for(j=0;j<c;j++)
        {
            printf("%lf  \n", matrix[i][j]);
        }
    }
}

int main()
{
	int i, j;
	int count=0; //tells us which thread

	srand(time(NULL));

	for(i=0;i<N;i++)
	{
		for(j=0;j<M;j++)
		{
			matrixA[i][j]=rand()%10;
		}
	}

	for(i=0;i<M;i++)
	{
		for(j=0;j<L;j++)
		{
			matrixB[i][j]=rand()%10;
		}
	}

	for(i=0;i<N;i++)
	{
		struct set *data = (struct set *)malloc(sizeof(struct set)); //memory allocate because of big matrix (faster)
		data->number=i; //MatrixA row
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&matrix_thread[count], &attr, thread, data); //pass data, create one thread for 1024*1024 matrix
		count++;
	}

	printf("The count of threads=%d\n", count);
	printf("MatrixA:\n");
	printMatrix(N,M,matrixA);
	printf("MatrixB:\n");
	printMatrix(M,L,matrixB);
	printf("MatrixC:\n");
	printMatrix(N,L,matrixC);

	for(i=0;i<count;i++)
	{
		pthread_join(matrix_thread[i],NULL);
	}

	return 0;
}
