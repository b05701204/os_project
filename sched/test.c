#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // fork()
#include <sched.h> // sched_setscheduler()

int values[] = {88, 56, 110, 2, 25};

int compare(const void * a, const void * b){
	return ( *(int*)a - *(int*)b );
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

#define policy(S) ((int)S%7%4)

int main(){
	int a = 'F';
	char s = 'F';
	printf("%d\n", a);
	printf("%d\n", policy(s));
	a = 'R';
	s = 'R';
	printf("%d\n", a);
	printf("%d\n", policy(s));
	a = 'S';
	s = 'S';
	printf("%d\n", a);
	printf("%d\n", policy(s));
	a = 'P';
	s = 'P';
	printf("%d\n", policy(s));
	printf("%d\n", s);
	
	
	printf("%d\n", SCHED_FIFO);
	printf("%d", SCHED_RR);
	
	qsort(values, 5, sizeof(int), compare);
	
	for(int n = 0; n < 5; ++n){
		printf("%d\n", values[n]);
	}
	
	//int a1[6] = {2, 5, 0, 3, 1, 4}, b[6];
	int a1[3] = {0, 1, 2}, b[3];
	inverse_permutation(a1, b, 3);
	printf("inverse\n");
	for(size_t i = 0; i < 3; ++i){
		printf("%d ", b[i]);
	}
	
	int va[10] = {78, 56, 32, 78, 67, 87, 34, 25, 90, 3};
	int in[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	
	printf("\n");
	sort(va, in, 10, 1);
	for(size_t i = 0; i < 10; ++i){
		printf("%d ", va[i]);
	}
	printf("\n");
	for(size_t i = 0; i < 10; ++i){
		printf("%d ", in[i]);
	}
	
	return 0;
}
