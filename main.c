#include <stdio.h>      /* puts, printf */
#include <time.h>
#include <unistd.h>


int main(){

	int target = 20;
	double DELAY_TIME = 10.0;

	clock_t time1;
        clock_t time2;
	double cpu_time_used =0;
	int sleeptime = 1;

	while(1){
		//measure cpu time 1
		time1 = clock();
		while(1){
			//measure cpu time 2
			time2 = clock();
			//time diff = t2 - t1
			cpu_time_used = ((double) (time2 - time1));
			printf("%f",cpu_time_used);
			printf("\n");
			if (cpu_time_used > DELAY_TIME){
				break;
			}
		}

	//calc sleep time
	sleeptime = cpu_time_used + DELAY_TIME;
	sleep(sleeptime);
	}

return 0;
}
