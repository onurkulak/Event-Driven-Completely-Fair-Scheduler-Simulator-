#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double exponentialRandom(double lambda){
    return -log(1- (rand() / (RAND_MAX + 1.0))) / lambda;
}

int main(int argv, char *argc[])
{
    int n = atoi(argc[1]);
    int startTime = atoi(argc[2]);
    int numBurst = atoi(argc[3]);
    int ioL = atoi(argc[5]);
    int cpuL = atoi(argc[4]);
    
    FILE *fpOut = fopen(argc[6],"w");
	

    srand((unsigned)time(NULL));
    for (int i=0; i<n; i++)
        {
        	int burstCount = (int)(exponentialRandom(1/(double)numBurst));
        	int st = (int)(exponentialRandom(1/(double)startTime));
        	int prio = (int)(rand() %40 ); //uniform prios
        	fprintf(fpOut, "%i start %i prio %i \n", i,st, prio);
        	for (int j = 0; j < burstCount-1; ++j)
        	{
        		fprintf(fpOut, "%i cpu %i\n", i,(int)(exponentialRandom(1/(double)cpuL)));
        		fprintf(fpOut, "%i io %i\n", i,(int)(exponentialRandom(1/(double)ioL)));
        	}
        	fprintf(fpOut, "%i cpu %i\n", i,(int)(exponentialRandom(1/(double)cpuL)));
        	fprintf(fpOut, "%i end\n", i);
        }
    fclose(fpOut);
    return 0;
}