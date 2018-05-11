#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#include <unistd.h> // fork()
#include <sched.h> // sched_setscheduler()

// wait for one unit of time
#define wait_unit(unit) ({ for(size_t i = 0; i < unit; ++i)wait_one_unit; })
// be careful, it is i++ not ++i!
#define wait_one_unit ({ volatile unsigned long i; for(i = 0; i < 1000000UL; i++); })
// convenient way to convert string into policy number
#define policy(S) ((int)S%7%4)

// We have to create SJF and PSJF as kernel module because they're not pre-built in kernel.
// The number 1, 2, 5, 6 must be recheck for enumeration of schedular/
// 1->FIFO, 2->RR, which is pre-vuilt in kernel.
// 5->SJF, 6->PSJF, need to set by us.
int POLICY[4] = {1, 2, 5, 6};
volatile unsigned long clock = 0;
volatile unsigned long remain = 0;
sem_t mutex, cond;

// sorting function for recording indices
// 1 for ascending, 0 for descending
void swap(int * a, int * b){
	int c = *a;
	*a = *b;
	*b = c;
}

void sort(int value[], int index[], int N, int ascend){
	if(ascend){
		for(size_t i = 0; i < N; ++i){
			for(size_t j = i + 1; j < N; ++j){
				if(value[i] > value[j]){
					swap(&value[i], &value[j]);
					swap(&index[i], &index[j]);
				}
			}
		}
	}
	else{
		for(size_t i = 0; i < N; ++i){
			for(size_t j = i + 1; j < N; ++j){
				if(value[i] < value[j]){
					swap(&value[i], &value[j]);
					swap(&index[i], &index[j]);
				}
			}
		}
	}
}

void inverse_permutation(const int a[], int b[], int N){
	int count = 0, cursor = 0, probe = 0;
	for(size_t i = 0; i < N; ++i)
		b[i] = -1;
	
	while(count < N){
		if(b[a[probe]] == -1){
			b[a[probe]] = probe;
			probe = a[probe];
			++count;
		}
		else{
			for(; cursor < N; ++cursor)
				if(b[cursor] == -1){
					probe = cursor;
					break;
				}
		}
	}
}

// flag == 1 for reading clock
// flag == 0 for reading remain
unsigned long reader(int flag){
	unsigned long temp;
	unsigned read_count = 0;
	
	// lock
	sem_wait(&mutex);
	if(++read_count == 1)
		sem_wait(&mutex);
	sem_post(&mutex);
	
	// critical section
	temp = (flag) ? clock : remain;
	
	// unclock
	sem_wait(&mutex);
	if(!--read_count)
		sem_post(&mutex);
	sem_post(&mutex);
	
	return temp;
}

unsigned long writer(unsigned long add){
	unsigned long temp;
	// lock
	sem_wait(&mutex);
	
	// critical section
	clock += add;
	temp = clock;
	
	// unclock
	sem_post(&mutex);
	
	return temp;
}

int main(){
	// parsing the input
	char S[4];
	int N;
	
	scanf("%s", S);
	scanf("%d", &N);
	
	char P[N][32];
	int R[N], T[N];
	int R_index[N], T_index[N], R_inverse[N], T_inverse[N];
	for(size_t i = 0; i < N; ++i)
		R_index[i] = T_index[i] = i;
	
	for(size_t i = 0; i < N; ++i){
		scanf("%s", P[i]);
		scanf("%d", &R[i]);
		scanf("%d", &T[i]);
	}
	
	// set the schedular
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	int s = sched_setaffinity(0, sizeof(mask), &mask);
	
	//int s;
	struct sched_param param;
	printf("Process main sched_setscheduler()\n");
	//param.sched_priority = sched_get_priority_min(POLICY[policy(S[0])]);
	//s = sched_setscheduler(0, POLICY[policy(S[0])], &param);
	
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	s = sched_setscheduler(0, SCHED_FIFO, &param);
	
	
	printf("Thread sched after %s\n", strerror(errno));
	assert(s == 0);

	// sort the ready and execution time, record the index in *_index
	sort(R, R_index, N, 1);
	sort(T, T_index, N, 1);
	inverse_permutation(R_index, R_inverse, N);
	inverse_permutation(T_index, T_inverse, N);

/*printf("\nR: ");
for(size_t i = 0; i < N; ++i){
	printf("%d ", R[i]);
}
printf("\nR_index: ");
for(size_t i = 0; i < N; ++i){
	printf("%d ", R_index[i]);
}
printf("\nR_inverse: ");
for(size_t i = 0; i < N; ++i){
	printf("%d ", R_inverse[i]);
}
printf("\nT: ");
for(size_t i = 0; i < N; ++i){
	printf("%d ", T[i]);
}
printf("\nT_index: ");
for(size_t i = 0; i < N; ++i){
	printf("%d ", T_index[i]);
}
printf("\nT_inverse: ");
for(size_t i = 0; i < N; ++i){
	printf("%d ", T_inverse[i]);
}
*/

	// init a mutex and conditional variable
	sem_init(&mutex, 0, 2);
	sem_init(&cond, 0, 2);
	
	// ready now
	pid_t pid;
	unsigned long ready_index = 0;

	for(size_t i = 0; i < N; ++i){
		if(!ready_index){
			wait_unit(R[0]);
			writer(R[0]);
		}
		do{
			sem_wait(&cond);
		}while(R[i] != reader(1));

		printf("%s forked.\n", P[R_index[i]]);
		pid = fork();
		
		if (pid == 0){
			// This is the child process.
			// Wait for execution time.
			for(size_t w = 0; w < T[T_inverse[R_index[i]]]; ++w){
				wait_one_unit;
				
				writer(1);
				sem_post(&cond);
			}
			printf("%s terminated.", P[R_index[i]]);
			exit(0);
		}
		else if (pid < 0){
			// The fork failed.
			printf("Fails.\n");
		}
		else{
			// This is the parent process.
			continue;
		}
	}
	wait_unit(1);
	
	int status;
	
	
	
	
	sem_destroy(&mutex);
	exit(0);
}

