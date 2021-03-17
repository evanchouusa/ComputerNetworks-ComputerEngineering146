/*
 * Name: Evan Chou
 * Date: Friday, 9/25/2020
 * Title: switch - C Program for switch vs packet
 * Description: This program computes the calculations for the comparisions between circuit switching and packet switching
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<math.h>

double x;

double factorial(x){
	if(x<=1)
		return 1;
	else
		return (x*factorial(x-1));
}

int main(void)
{
	int linkBandwidth = 200; //bandwidth of network link
	int userBandwidth = 20; //bandwidth required for given user
	int nCSusers; //number of circuit switching users 
	int nPSusers = 19; //number of packet switching users
	double tPSusers = 0.10; //percentage of time packet switching user need to transmit
	double pPSusersBusy; //probability that a given packet switching user is busy transmitting
	double pPSusersNotBusy; //probability that one packet switching user is not busy transmitting
	double pPSusers;

	double c;
	double d;
	double e;
	double f;
	double g;
	double h;

	//Circuit switching scenario
	nCSusers = linkBandwidth/userBandwidth; 

	//Packet switching scenario
	pPSusers = tPSusers;
	pPSusersNotBusy = 1-pPSusers;
	c = pow(1-pPSusers, nPSusers-1);
	d = pPSusers * pow(pPSusersNotBusy, nPSusers-1);
	e = nPSusers * (pPSusers * pow(pPSusersNotBusy, nPSusers-1));
	f = pow(nPSusers, 10) * pow(pPSusersNotBusy, nPSusers-10);
	g = ((factorial(nPSusers))/((factorial(10)) * (factorial(nPSusers-10)))) * pow(pPSusers, 10) * pow(pPSusersNotBusy, nPSusers-10);
	
	h = 0.0;
	int i;
	for(i=11; i<=nPSusers; i++)
	{
		h += (factorial(nPSusers)/(factorial(i) * factorial(nPSusers-1))) * pow(pPSusers, i) * pow(pPSusersNotBusy, nPSusers-i);
	}

	//Solution to circuit switching scenario
	printf("%d is the number of circuit-switched users that can be supported\n", nCSusers);

	//Solutions to packets switching scenario
	printf("%f is the probability that a given user is busy transmitting\n", pPSusers);
	printf("%f is the probability that one specfic other use is not busy\n", pPSusersNotBusy);
	printf("%f is the probability that all of the other specfic other users are not busy\n", c);
	printf("%f is the probability that one specific user is transmitting and the remaining users are not transmitting\n", d);
	printf("%f is the probability that exactly on eof the nPSusers is busy\n", e);
	printf("%e is the probability that 10 specific users of nPSusers are transmitting and the others are idle\n", f);
	printf("%e is the probability that any 10 users of nPSusers are transmitting and the others are idle\n", g);
	printf("%e is the probability that more than 10 users of nPSusers are transmitting and the others are idle\n", h);

	return 0;
}
