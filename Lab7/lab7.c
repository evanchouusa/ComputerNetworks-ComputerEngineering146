/*
 * Name: Evan Chou
 * Date: November 6, 2020
 * Title: Lab7 - Link state routing
 * Description: This program develops a linke state routing algorithm and runs the LS routing for a network
 */

//make sure to include all needed libraries
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#define	N			4
#define	INFINITE	1000

//define struct types
typedef struct machines {
	char name[50];
	char ip[50];
	int port;
} MACHINES;

//declare global variables
MACHINES	machines[N];
int			costs[N][N];
int			distances[N];
int			myid, nodes; //this is router id, and router port
int			sock;
struct sockaddr_in addr;
struct sockaddr_in otheraddr;
socklen_t addrLen = sizeof(struct sockaddr);
pthread_mutex_t	lock;

// functions
void *run_link_state (void *);
void *receive_info (void *);
void *update_costs (void *); //we need to add this function to update our costs
void print_costs (void);

/**********************
 * main
 **********************/
int main (int argc, char *argv[]) {
	FILE	*fp;
	int		i, j;
	pthread_t	thr1, thr2, thr3;
	int		id, cost;
	int		packet[3];

	//check number of arguments
	if (argc < 5) {
        printf("There is an argument number error\n");
        return 1;
	}
	
	//set router id and router port
	myid = atoi(argv[1]);
	nodes = atoi(argv[2]);

	if (myid >= N) {
		printf ("wrong id\n");
		return 1;
	}

	if (nodes != N) {
		printf("wrong number of nodes\n");
		return 1;
	}

	//get info on machines from the file using fopen and fscanf and populate the array of machines-struct

	// get costs from the costs file and populate the matrix
	if ((fp = fopen (argv[3], "r")) == NULL) { //open costfile.txt and read file if it is not equal to NULL
        printf("Failure to read the file\n"); //if equal to null, means reading file failed
		return 1;
	}

	for(i = 0; i < N; i++) {
	    for(j = 0; j < N; j++) {
            fscanf(fp, "%d", &costs[i][j]); 
        }
    }
	fclose(fp); //close file

    fp = fopen (argv[4], "r"); //we are opening file to read and actually getting the costs from here
	for (i = 0; i < N; i++) {
	    fscanf(fp, "%s %s %d", machines[i].name, machines[i].ip, &machines[i].port);
	}
	fclose(fp); //close file

    // init address
    addr.sin_family = AF_INET;
    addr.sin_port = htons(machines[myid].port);
    addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Endpoint Socket Setup Failed\n");
        exit(1);
    }

    // bind
    if((bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr))) < 0) {
        perror("Failure to bind server address to the endpoint socket\n");
        exit(1);
    }

	// create threads and initiate mutex
	pthread_mutex_init (&lock, NULL);

   	pthread_create(&thr1, NULL, receive_info, NULL);
    pthread_create (&thr2, NULL, update_costs, NULL);
    pthread_create (&thr3, NULL, run_link_state, NULL);

	//read changes from the keyboard, update the costs matrix inform other nodes about the changes
    pthread_join(thr1, NULL);
	pthread_join(thr2, NULL);
    pthread_join(thr3, NULL);

	sleep (5);

}


/**********************
 * receive info
 **********************/
//If there is a cost update on other machine, they will let us know through a UDP packet. We need to receive that packet and update our costs table
void *receive_info (void *arg) {
    printf("receive_info\n"); //tell users that they have reached this function
	int	new[3];
	int	nbytes;
	int	n0, n1, n2;

	socklen_t len = sizeof(otheraddr);

	while(true) {
		printf("got to receive from\n");

	    int n = recvfrom(sock, &new, sizeof(new), 0, (struct sockaddr*) &otheraddr, &len); //receiving data from node that was updated

		//update costs table for all nodes
		if(n>0) {
			n0 = new[0];
        	n1 = new[1];
        	n2 = new[2];

        	printf("Received new update, trying to update\n");

			//updating cost tables
        	pthread_mutex_lock(&lock);
        	costs[n0][n1] = n2;
        	costs[n1][n0] = n2;
        	pthread_mutex_unlock(&lock);

        	printf("Updated costs:\n");
        	print_costs();
		}
    }
}

//function updates the cost for single node
void *update_costs (void *arg) {
    printf("update_cost\n");
    int neighborId;
    int newCost;

	//update node cost by entering neighbor node's id and distance you want to change
    while(1) {
        printf("Enter neighbor's id and new cost: \n");
        scanf("%d %d", &neighborId, &newCost);

		//updating cost table
        pthread_mutex_lock(&lock);
        costs[myid][neighborId] = newCost;
        costs[neighborId][myid] = newCost;
        pthread_mutex_unlock(&lock);

        printf("Updated table\n");
        print_costs();

        int i;

        for(i = 0; i < N; i++) {
            if(i == myid) {
                printf("my router\n");
                continue;
            }

			otheraddr.sin_family = AF_INET;
            otheraddr.sin_port = htons(machines[i].port);
            inet_pton(AF_INET, machines[i].ip, &otheraddr.sin_addr.s_addr);

            int info[3] = {
                    myid,
                    neighborId,
                    newCost
            };
            printf("Updating %s \t %d \n", machines[i].ip, machines[i].port);

			//send updated information to the other nodes, so they can receive and update their cost table
            int n = sendto(sock, info, sizeof(info), 0, (struct sockaddr*) &otheraddr, addrLen);
            printf("sent\n");
        }
		sleep(9);
    }
}

/**********************
 * run_link_state
 **********************/
// This thread runs the Dijkastra's algorithm on the cost table and calculates thte shortest path
void *run_link_state (void *arg) {
	int	taken[N]; 
	int	min, spot, count;
	int	i, v;
	int dist[N]; //array of shortest distances

	//I want to print out the original cost table just for reference to see if the new update cost table actually changed
	printf("original cost\n");
	print_costs();

	//In this while loop, we are updating the new shortest distance
	while (1) {

		for (i = 0; i < N; i++) {
        	dist[i] = INFINITE;
        	taken[i] = 0;
		}
		dist[myid] = 0;

		for (count = 0; count < N - 1; count++) {
			min = INFINITE;
			spot = -1;
			for (v = 0; v < N; v++) {
					//If new distance is shorter than the minimum, then new distance is now the minimum
			        if (taken[v] == 0 && dist[v] <= min) {
            			min = dist[v];
            			spot = v;
					}
			}
			taken[spot]=1;

			//using the formula provided in the slides to updata and calculate newest distance
			for (v = 0; v < N; v++) {
	            if (taken[v]==0 && costs[spot][v]!=0 && dist[spot] != INFINITE && dist[spot] + costs[spot][v] < dist[v]) {
                	dist[v] = dist[spot] + costs[spot][v];
				}

			}
		}

		//printing new shortest distance for specific node
		printf ("new-shortest distances:\n");
		for (i = 0; i < N; i++) {
			printf("%d ", dist[i]);
		}
		printf ("\n");
	    sleep(8);
	}
}



/**********************
 * print costs
 **********************/
//NOTE: Did not change anything at all for this function. Kept professors barebone function as is.
void print_costs (void) {
	int i, j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			pthread_mutex_lock (&lock);
			printf ("%d ", costs[i][j]);
			pthread_mutex_unlock (&lock);
		}
		printf ("\n");
	}
}

