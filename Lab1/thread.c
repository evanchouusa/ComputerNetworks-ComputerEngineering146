/* Name: Evan Chou
 * Date: Friday, 9/25/2020
 * Title: thread - C Program using 2 threads
 * Description: This program uses threads to show delay through various arguments
 */

#include <stdio.h>      /* printf, stderr */
#include <sys/types.h>  /* pid_t */
#include <unistd.h>     /* fork */
#include <stdlib.h>     /* atoi */
#include <errno.h>      /* errno */ 
#include <math.h>		/* math*/
#include <pthread.h>	/* pthread */

void *pass(void *arg)
{
	usleep(*(int *)arg);
	return(NULL);
}

/* main function with command-line arguments to pass */
int main(int argc, char *argv[]){
    pthread_t tid;
    int i, n = atoi(argv[1]); // n microseconds to input from keyboard for delay
	for(i=0;i<100;i++)
	{
		pthread_create(&tid, NULL, pass, &n);
	}
	for(i=0;i<100;i++)
	{
		pthread_join(tid, NULL);
		printf("thread number %d is returned\n", i);
	}
	return 0;
}
