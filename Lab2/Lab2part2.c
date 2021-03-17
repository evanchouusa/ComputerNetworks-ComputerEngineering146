/*
 * Evan Chou
 * Lab2 Part 2
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int n=atoi(argv[1]); //n parallel TCP connections
	int nonper;
	
	int i;
	int RTT0, RTT1, RTT2, RTTHTTP, RTTDNS;

	RTT0=3;
	RTT1=20;
	RTT2=26;
	RTTHTTP=47;

	RTTDNS = RTT0 + RTT1 + RTT2;

	//Answer to Step 4 a.
	printf("\n%d msec\n", RTTDNS+2*RTTHTTP);

	//Answer to Step 4 b.
	printf("\n%d msec\n", RTTDNS+2*RTTHTTP+2*6*RTTHTTP);

	//if n>6 true, nonper=1, else 6/n
	if(n>6)
	{
		nonper=1;
	}
	else
	{
		nonper=6/n;
	}

	if((n<6)&&(6%n!=0))
	{
		nonper+=1; //request/ask for another connection
	}

	//Answer to Step 4 c.
	printf("\nParallel connection number: %d, Persistent Response Time: %d msec\n", n, RTTDNS+2*RTTHTTP+nonper*RTTHTTP);
	
	printf("\nParallel connection number: %d, Non-Persistent Response Time: %d msec\n", n, RTTDNS+2*RTTHTTP+2*nonper*RTTHTTP);

	return 0;
}

