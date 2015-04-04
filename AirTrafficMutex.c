/*
* Created by: Asad Zia
* 
* Description:
* An air traffic control simulation using mutexes.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUMBER_OF_RUNWAYS 1
#define NUMBER_OF_GATES 2
#define NUMBER_OF_AIRPLANES 3
#define TRUE 1 					// used for setting up the conditions in the mutexes
#define FALSE 0					// used for setting up the conditions in the mutexes

int gate_array[NUMBER_OF_GATES];		/* the array holds all the gates available */
int runway_array[NUMBER_OF_RUNWAYS];	/* the array holds all the runways available */

/*
* A data-structure used to model the airplanes
*/
typedef struct {
	int id; 		/* The ID of the airplane */
	pthread_t tid; 	/* The thread ID of the airplane */
} airplanes;

pthread_mutex_t runway_lock;	/* the mutex lock for the runway */
pthread_mutex_t gate_lock;		/* the mutex lock for the gates */

/*
* The Function for setting up the runways for the planes. The values in the array are set with 0 initially.
* Value of 0 = Runway is empty
* Value of 1 = Runway is not empty (This is set later on in the functions which utilize the runway_array)
*/
static void runway_init(int arr[NUMBER_OF_RUNWAYS]) {
	int i;

	for (i = 0; i < NUMBER_OF_RUNWAYS; i++) {
		arr[i] = 0;
	}
}

/*
* The Function for setting up the runways for the planes. The values in the array are set with 0 initially.
* Value of 0 = Gate is empty
* Value of 1 = Gate is not empty (This is set later on in the functions which utilize the gate_array)
*/
static void gate_init(int arr[NUMBER_OF_GATES]) {
	int i;

	for (i = 0; i < NUMBER_OF_GATES; i++) {
		arr[i] = 0;
	}
}


/*
* The function called by a thread to ask for landing
* It basically checks if the value of any runway in the runway_array is set to 0 
* (if its 0, then the runway is empty, if its 1 then its not empty)
*/
static void landing_function(int id) {
	printf("[AC]: Flight %d requesting landing\n", id);

	pthread_mutex_lock(&runway_lock);			// entering the critical section
	int i;
	int stop = FALSE;	
		while (stop == FALSE) {					// setting condition variables to break out of the loops
			for (i = 0; i < NUMBER_OF_RUNWAYS; i++) {
				if (runway_array[i] == 0) {		// if the value assigned to a runway is 0, then its free
					stop = TRUE;				// change the value of the conditional variable
					runway_array[i] = id;		// assign the ID of the plane to the runwy which is going to be used
					break;
				}
			}
			if (stop == TRUE){
				break;
			}
		}
	printf("[GC]: Airplane %d assigned runway %d\n", id, i + 1);
	pthread_mutex_unlock(&runway_lock);			// leaving the critical section

	printf("[AC]: Airplane %d has landed\n", id);
}

/*
* The function called by a thread to proceed to the gate
* It basically checks if the value of any gate in the gate_array is set to 0
* (if its 0, then the gate is empty, if its 1, then its not empty)
*/
static void proceed_to_gate(int id) {
	printf("[AC]: Airplane %d requesting gate\n", id);
	pthread_mutex_lock(&gate_lock);					// entering the critical section
	int i, j;
	int stop = FALSE;

		while (stop == FALSE) {						// setting condition to break out of the loops
			for (i = 0; i < NUMBER_OF_GATES; i++) {
				if (gate_array[i] == 0) {			// if the value assigned to a runway is 0, then its free
					stop = TRUE;					// change the value of the conditional variable
					gate_array[i] = id;				// assign the ID of the plane to the runway which is going to be used
					break;
				}
			}
			if (stop == TRUE){
				break;
			}
		}

		// free the runway by setting the runway value for a particular airplane to 0
		for (j = 0; j < NUMBER_OF_RUNWAYS; j++) {
			if (runway_array[j] == id) {
				runway_array[j] = 0;
				break;
			}
		}

	printf("[GC]: Airplane %d assigned gate %d\n", id, i + 1);
	pthread_mutex_unlock(&gate_lock);				// leaving the critical section
}

/*
* The function called by a thread to proceed to the hangar
* I have set a default waiting time of 10 seconds before the airplane goes to the hangar
*/
static void proceed_to_hangar(int id) {
	int i;
	sleep(10);
	printf("[AC]: Airplane %d heading to hangar\n", id);

	// Empty the gate by setting the value of the gate in the gate_array to 0 (gate is empty now)
	for (i = 0; i < NUMBER_OF_GATES; i++) {
		if (gate_array[i] == id) {
			gate_array[i] = 0;
			break;
		}
	}
}

/*
* This function is used by the airplane threads. Basically, it calls the functions
* for landing and assigning gates and runways.
*/
static void* airplane_control(void* args) {
	airplanes* plane = (airplanes*)args;
	landing_function(plane->id);
	proceed_to_gate(plane->id);
	proceed_to_hangar(plane->id);
	return NULL;
}

int main(int argc, char* argv[]) {
	int i, status;
	airplanes* planes;

	runway_init(runway_array);
	gate_init(gate_array);

	// initialize mutexes
	if (pthread_mutex_init(&runway_lock, NULL) != 0 || pthread_mutex_init(&gate_lock, NULL) != 0)
    {
        printf("Mutex init failed\n");
        return 1;
    }

    // allocate memory for the plane data structure
	planes = (airplanes *)calloc(NUMBER_OF_AIRPLANES, sizeof(airplanes));
	if (!planes) {
		printf("Error in memory Allocation!\n");
		exit(1);
	}

	// creating the threads
	for (i = 0; i < NUMBER_OF_AIRPLANES; i++) {
		planes[i].id = i + 1;
		status = pthread_create(&planes[i].tid, NULL, airplane_control, (void*)&planes[i]);

		if (status != 0) {
			printf("Error in function pthread_create()!\n");
		}
	}

	// joining the threads
	for (i = 0; i < NUMBER_OF_AIRPLANES; i++) {
		pthread_join( planes[i].tid, NULL);
	}

	// freeing the memory for the plane data-structure
	(void) free(planes);

	// destorying the mutexes
	if (pthread_mutex_destroy(&runway_lock) || pthread_mutex_destroy(&gate_lock)) {
		printf("Unable to destory Mutexes properly!\n");
		exit(1);
	}
	return 0;
}
