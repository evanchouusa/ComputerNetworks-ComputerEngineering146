/*
 * Name: Evan Chou
 * Title: Lab3 Part 1
 * Description: This program computes a file transfer of a text or binary file
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(int argc, char *argv[])
{
	char *filein = argv[1];
	char *fileout = argv[2];

	FILE *fpr, *fpw;
	char buffer[1000];


	char comp[5];
	strncpy(comp, filein + (strlen(filein) -4), 4);
	comp[4] = '\0';

	fpr = fopen(filein, "r");
	fpw = fopen(fileout, "w");

	if(strcmp(comp, ".txt")==0)
	{
		while(!feof(fpr))
		{
			fscanf(fpr, "%s", buffer);
			fprintf(fpw, "%s\n", buffer);
			memset(buffer, 0, sizeof buffer);
		}   
	}
	else
	{
		while(!feof(fpr))
		{
			fread(&buffer, sizeof(buffer), 1, fpr);
			fwrite(&buffer, sizeof(buffer), 1, fpw);
			memset(buffer, 0, sizeof buffer);
		}
	}	
	fclose(fpr);
	fclose(fpw);
}
