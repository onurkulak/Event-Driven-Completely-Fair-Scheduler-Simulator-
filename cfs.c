//because of the complexity of the project
//it was hard to understand at  first
//so new requirements added in a non-elegant way
//now this code turned into a clusterfuck
// All hope abandon, ye who enter here

#include <stdio.h>
#include <stdlib.h>

 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

static const int prio_to_weight[40] = {
/* -20 */ 88761, 71755, 56483, 46273, 36291,
/* -15 */ 29154, 23254, 18705, 14949, 11916,
/* -10 */ 9548, 7620, 6100, 4904, 3906,
/* -5 */ 3121, 2501, 1991, 1586, 1277,
/* 0 */ 1024, 820, 655, 526, 423,
/* 5 */ 335, 272, 215, 172, 137,
/* 10 */ 110, 87, 70, 56, 45,
/* 15 */ 36, 29, 23, 18, 15,
};
typedef enum { false, true } bool;

typedef struct statistic
{
	int pid;
	int prio;
	long long st, ft, tt, waitTime, rt;
} statistic;

typedef struct process
{
	int pid;
	long long* bursts;
	int burstCounter;
	int burstSize;
	long long burstLeft;
	long long startTime;
	long long * waits;
	long long vtime;
	int prio;
	bool touchedBurst;
} process;


typedef struct event
{
	int eventType;
	/* 0 = new process
	1 = time slice end
	2 = i/o return
	3 = burst end
	4 = termination
	*/
	long long time;
	process* prc;
} event;
typedef struct node node;
typedef struct node
{
	long long key;
	void* data;
	node *next;
} node;

typedef struct queue
{
	node* head;
} queue;

void insert(node* n, queue *q){
	if(q->head == 0){
		q->head = n;
		n->next = 0;
	}
	else if(n->key<q->head->key) {
		n->next = q->head;
		q->head = n;
	}
	else{
		node* iter1, *iter2;
		iter1 = q->head;
		iter2 = iter1 -> next;
		while(iter2!=0&&n->key>=iter2->key){
			iter2 = iter2 -> next;
			iter1 = iter1 -> next;
		}
		n->next = iter2;
		iter1->next = n;
	}
}

bool retrieve(queue* q, node** n){
	if(q->head==0){
		return false;
		}
	else{
		(*n) = q->head;
		q->head = q->head->next;
		return true;
	}
}
FILE* fpOut;
void handleEvent(event* n);
void getInputs(queue *events, char* input);
void addEvent(queue *q, event* e);
void reschedule(event* n);
void updateVirtualTime();
void newProcess(event* n);
void IOReturn(event* n);
void burstEnd(event* n);
void termination(event* n);
void preempt();
long long time;
long long minVirtualTime;
long long lastSchedulingTime;
queue stats;
queue processes;
queue events;
int numProcess = 0;
int totalWeight = 0;
process* runningProcess;
void printResults();

int main(int argc, char const *argv[])
{
	processes.head = 0;
	events.head = 0;
	stats.head = 0;
	runningProcess = 0;
	time = 0;
	minVirtualTime = 0;
	lastSchedulingTime = 0;
	
	fpOut = fopen(argv[2],"w");
	
	getInputs(&events, argv[1]);
	node** n = malloc(sizeof(node*));
	while(retrieve(&events, n)){
		handleEvent(((event*)((*n)->data)));
	}
	printResults();
	fclose(fpOut);
	return 0;
}

void handleEvent(event* n){
	if(runningProcess!=0){
		runningProcess->burstLeft-=n->time-time;
		runningProcess->vtime += (n->time-time)*((double)prio_to_weight[20]/prio_to_weight[runningProcess->prio]);
	}
	updateVirtualTime();
	//printf("Event: %i\tPid: %i\tTime: %lli\n",n->eventType, n->prc->pid, n->time );
	if(n->eventType==0)
		newProcess(n);
	else if(n->eventType==2)
		IOReturn(n);
	else if(n->eventType==3)
		burstEnd(n);
	else if(n->eventType==4)
		termination(n);
	
	
	if(n->eventType!=1||n->time%10000000LL==0){
		//printf("Resik\n");	
		reschedule(n);
		//printf("Resikked\n");
	}else{
		n->time = (n->time/10000000LL+1)*10000000LL;
		addEvent(&events,n);
		//free(n);
	}
	time = n->time;
}
void reschedule(event* n){
	bool processChange = false;
	process* runningTemp = runningProcess;
	if(n->eventType==3||n->eventType==4){
		runningProcess = 0;
		processChange = true;
	}


	if(runningProcess!=0)
	{
		node* nP = malloc(sizeof(node));
		nP->key = runningProcess->vtime+runningProcess->pid;
		nP->data = runningProcess;
		insert(nP, &processes);
		node** nPt = malloc(sizeof(node*));
		retrieve(&processes,nPt);
		if ((*nPt)->data == nP->data)
		{
			processChange = false;
			//printf("no process change\n"); 
		}
		else{
			//printf("new process\n"); 
			processChange = true;
			runningProcess = (process*)((*nPt)->data);
		}
		free (*nPt);free (nPt);
	}

	
	
	if(runningProcess==0){
		node** nPt = malloc(sizeof(node*));
		if(retrieve(&processes,nPt)){
			//printf("filling empty area\n"); 
			runningProcess = (process*)((*nPt)->data);
			processChange = true;
			free (*nPt);
		}
		free (nPt);
	}

	//printf("preempting\n");
	preempt();
	 
	
	if(runningProcess!=0){
		//printf("calculating time slice\n"); 
		long long timeSlice = 1000000LL*max(numProcess*10,200);
		event* e = malloc(sizeof(event));
		timeSlice *= prio_to_weight[runningProcess->prio]/(double)totalWeight; 
		timeSlice = max(timeSlice,10000000LL);
		if(timeSlice>=runningProcess->burstLeft){
			timeSlice = runningProcess->burstLeft;
			if(runningProcess->burstCounter==runningProcess->burstSize-1)
				e->eventType = 4;
			else e->eventType = 3;
		}
		else e->eventType = 1;
		//printf("%i time slice %lli\n",runningProcess->pid,timeSlice); 
	 
		e->time = n->time+timeSlice;
		e->prc = runningProcess;
		addEvent(&events,e);
	 
		if(!runningProcess->touchedBurst){
			runningProcess->touchedBurst = true;
			runningProcess->waits[runningProcess->burstCounter/2] = n->time-runningProcess->waits[runningProcess->burstCounter/2];
	 
		}
	 
	}
	 
	if (processChange&&n->time-lastSchedulingTime>0)
	{
		if(runningTemp==0)
			fprintf(fpOut,"idle %lli\n",  (n->time-lastSchedulingTime)/1000000LL);
		else 	fprintf(fpOut,"%i %lli\n", runningTemp->pid, (n->time-lastSchedulingTime)/1000000LL);
		lastSchedulingTime = n->time;
	}
	
}
void updateVirtualTime(){
	long long t1 = 0;long long t2 = 0;
	if (processes.head!=0){// if there is a ready process
        t1 = ((process*)(processes.head->data))->vtime;
    } 
    if (runningProcess!=0)
    {
    	t2 = runningProcess->vtime;
    }
    minVirtualTime = max(minVirtualTime, min(t1,t2));
}

void newProcess(event* n){
	updateVirtualTime();
	n->prc->vtime = max(0,minVirtualTime-10000000LL);
	n->prc->waits[0] = n->prc->startTime;
	node* nP = malloc(sizeof(node));
	nP->key = n->prc->vtime+n->prc->pid;
	nP->data = n->prc;
	insert(nP, &processes);
	numProcess++;
	totalWeight += prio_to_weight[n->prc->prio];
}


void IOReturn(event* n){
	n->prc->burstCounter++;
	n->prc->burstLeft = n->prc->bursts[n->prc->burstCounter];
	n->prc->waits[n->prc->burstCounter/2] = n->time;
	n->prc->touchedBurst = false;
	node* nP = malloc(sizeof(node));
	nP->key = n->prc->vtime+n->prc->pid;
	nP->data = n->prc;
	insert(nP, &processes);
	numProcess++;
	totalWeight += prio_to_weight[n->prc->prio];
}

void burstEnd(event* n){
	event* e = malloc(sizeof(event));
	n->prc->burstCounter++;
	e->time = n->time+n->prc->bursts[n->prc->burstCounter];
	e->prc = n->prc;
	e->eventType = 2;
	addEvent(&events, e);
	numProcess--;
	totalWeight -= prio_to_weight[n->prc->prio];
}

void termination(event* n){
	node* nodep = malloc(sizeof(node));
	statistic* stat = malloc(sizeof(statistic));
	nodep->data = stat;
	nodep->key = n->prc->pid;

	stat->pid = n->prc->pid;
	stat->prio = n->prc->prio;
	stat->st = n->prc->startTime/1000000LL;
	stat->ft = n->time/1000000LL;
	stat->tt = stat->ft-stat->st;
	stat->waitTime = stat->tt;
	for(int i = 0; i < n->prc->burstSize; i++)
		stat->waitTime-=n->prc->bursts[i]/1000000LL;
	stat->rt = 0;
	for (int i = 0; i < n->prc->burstSize/2+1; ++i)
		stat->rt+=n->prc->waits[i];
	stat->rt/=1000000LL*(n->prc->burstSize/2+1);
	numProcess--;
	totalWeight -= prio_to_weight[n->prc->prio];
	insert(nodep,&stats);
}


void getInputs(queue *events, char* input){
	FILE *fp=fopen(input,"r");
	int pCount = 1;
	int lastPid; int curPid;
	fscanf(fp,"%i%*[^\n]", &lastPid);
	while(fscanf(fp,"%i%*[^\n]", &curPid)!=EOF){
		if(curPid!=lastPid){
			pCount++;
			lastPid = curPid;
		}
	}
	fclose(fp);
	

	fp=fopen(input,"r");
	int* burstCounts = (int*)malloc(pCount*sizeof(int));
	for(int i = 0; i < pCount; i++)
		burstCounts[i]=-1;
	int pCounter = 0;
	fscanf(fp,"%i%*[^\n]", &lastPid);
	while(fscanf(fp,"%i%*[^\n]", &curPid)!=EOF){
		if(curPid==lastPid)
			burstCounts[pCounter]++;
		else {
			pCounter++;
			lastPid = curPid;
		}
	}

	process* pHeap = malloc(sizeof(process)*pCount);
	event* eHeap = malloc(sizeof(event)*pCount);
	fclose(fp);


	fp=fopen(input,"r");
	for(int i = 0; i < pCount; i++){
		fscanf(fp,"%i start %lli prio %i", &pHeap[i].pid, &pHeap[i].startTime, &pHeap[i].prio);
		pHeap[i].bursts = malloc(sizeof(long long)*burstCounts[i]);
		pHeap[i].burstSize = burstCounts[i];
		pHeap[i].burstCounter = 0;
		pHeap[i].startTime *= 1000000LL;
		pHeap[i].waits = malloc(sizeof(long long)*(burstCounts[i]+1)/2);
		for(int j = 0; j < burstCounts[i]; j++){
			fscanf(fp,"%i%*s%lli", &pHeap[i].pid, &pHeap[i].bursts[j]);
			pHeap[i].bursts[j] *= 1000000LL;
		}
		fscanf(fp,"%*i end");
		pHeap[i].burstLeft = pHeap[i].bursts[0];
		pHeap[i].touchedBurst = false;

		eHeap[i].eventType = 0;
		eHeap[i].time = pHeap[i].startTime;
		eHeap[i].prc = &pHeap[i];
		addEvent(events, &eHeap[i]);
	}
	fclose(fp);
	free(burstCounts);
}

void addEvent(queue *q, event* e)
{
	node* n = malloc(sizeof(node));
	n->key = e->time;
	n->data = e;
	insert(n,q);
}
void preempt(){
	while(events.head!=0&&(((event*)((events.head)->data))->eventType==1||
			((event*)((events.head)->data))->eventType==3||
			((event*)((events.head)->data))->eventType==4))
	{
			event* deleteTemp = (event*)((events.head)->data);
			node* deleteTemp2 = events.head;
			events.head = events.head->next;
			free(deleteTemp);
			free(deleteTemp2);
	}
	 

	for(node* tempN = events.head; tempN != 0 && tempN->next != 0; tempN = tempN->next){
		if(((event*)(tempN->next->data))->eventType==1||
		((event*)(tempN->next->data))->eventType==3||
		((event*)(tempN->next->data))->eventType==4){
			event* deleteTemp = (event*)(tempN->next->data);
			node* deleteTemp2 = tempN->next;
			tempN->next = tempN->next->next;
			free(deleteTemp);
			free(deleteTemp2);
		}
	}
}


void printResults(){
	node** n = malloc(sizeof(node*));
	while(retrieve(&stats, n)){
		statistic* s = ((statistic*)((*n)->data));
		printf("%i %i %lli %lli %lli %lli %lli\n", s->pid, s->prio, s->st, s->ft, s->tt, s->waitTime, s->rt);
	}
}