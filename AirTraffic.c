/*
*
* A air traffic control simulation developed by Asad Zia.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NUMBER_OF_RUNWAYS 2
#define NUMBER_OF_GATES 1
#define NUMBER_OF_AIRPLANES 9
#define TRUE 1 					// used for setting up the conditions in the mutexes
#define FALSE 0					// used for setting up the conditions in the mutexes

int gate_array[NUMBER_OF_GATES];			/* the array holds all the gates available */
int runway_array[NUMBER_OF_RUNWAYS];		/* the array holds all the runways available */
int taxiway_array[NUMBER_OF_RUNWAYS][3]; 	/* the array holding all the taxiways available for each runway */

/*
* A data-structure used to model the airplanes
*/
typedef struct {
	int id; 		/* The ID of the airplane */
	pthread_t tid; 	/* The thread ID of the airplane */
} airplanes;

sem_t runway_sem;       					/* The semaphore used for the runways */
sem_t gate_sem;    	 					    /* The semaphore used for the gates */
sem_t queue_sem[NUMBER_OF_RUNWAYS];         /* The semaphore used for the gate queue */

/*
* The Function for setting up the runways for the planes. The values in the array are set with 0 initially.
* Value of 0 = Runway is empty
* Value > 0 = Runway is not empty (This is set later on in the functions which utilize the runway_array)
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
* Value > 0 = Gate is not empty (This is set later on in the functions which utilize the gate_array)
*/
static void gate_init(int arr[NUMBER_OF_GATES]) {
	int i;

	for (i = 0; i < NUMBER_OF_GATES; i++) {
		arr[i] = 0;
	}
}

/*
* The Function for setting up the taxiways for the planes. The values in the array are set with 0 initially.
* Value of 0 = taxiway is empty
* Value > 0 = Taxiway is not empty (This is set later on in the functions which utilize the taxiway_array)
*/
static void taxiway_init(int arr[NUMBER_OF_RUNWAYS][3]) {
	int i,j;

	for (i = 0; i < NUMBER_OF_RUNWAYS; i++) {
		for (j = 0; j < 3; j++){
			arr[i][j] = 0;
		}
	}
}


/*
* The function called by a thread to ask for landing
* It basically checks if the value of any runway in the runway_array is set to 0 
* (if its 0, then the runway is empty, if its > 0 then its not empty)
*/
static void landing_function(int id) {
	int sval, i;
	printf("[AC]: Flight %d requesting landing\n", id);
	sem_wait(&runway_sem);
	int stop = FALSE;

		/* in this while loop, check if the taxiways connected to the runway are empty to allow landing on that particular runway */
		while (stop == FALSE) {
			for (i = 0; i < NUMBER_OF_RUNWAYS; i++) {
				sem_getvalue(&queue_sem[i], &sval);
				if (runway_array[i] == 0 && (taxiway_array[i][0] == 0 || taxiway_array[i][1] == 0 || taxiway_array[i][2] == 0) && sval > 0) {	
					runway_array[i] = id;					// assign the ID of the plane to the runwy which is going to be used
					stop = TRUE;
					break;
				}
			}
			if (stop == TRUE) {
				break;
			}
		}

	printf("[GC]: Airplane %d assigned runway %d\n", id, i + 1);
	printf("[AC]: Airplane %d has landed\n", id);
}

/*
* the function for proceeding to the taxiways
* if taxiway_array has value 0 in it, it is empty for that index
* if it has a value > 0 then its occupied
*/
static void proceed_to_taxiways(int id) {
	 int sval, val, j,k, runway;
	 int stop = FALSE;
    
    /* check the associated runway for a particular aircrapft */
    for (j = 0; j < NUMBER_OF_RUNWAYS; j++) {
			if (runway_array[j] == id) {
				runway = j;
				break;
			}
		}

	/* in this loop, move the plane to a taxiway and free the runway by setting its value to 0 */
     for (j = 0; j < NUMBER_OF_RUNWAYS; j++) {
     	for (k = 0; k < 3; k++) {
			if (taxiway_array[j][k] == 0 && runway_array[j] == id) {
				sem_wait(&queue_sem[j]);		/* decrement semaphore for taxiway queue */
				sem_getvalue(&queue_sem[j], &sval);
				taxiway_array[j][k] = id;		/* occupy empty taxiway */

				runway_array[j] = 0;			/* free runway by setting it to 0 to allow other planes to land */
				sem_post(&runway_sem);			/* indicate that runway is open */
				sem_getvalue(&runway_sem, &val);
				stop = TRUE;
				break;
			}
			if (stop == TRUE) {
				break;
			}
  		}   
	} 
	printf("[AC]: Airplane %d queing in taxiway of runway %d\n", id, runway + 1);    
}

/*
* The function called by a thread to proceed to the gate
* It basically checks if the value of any gate in the gate_array is set to 0
* (if its 0, then the gate is empty, if its > 0, then its not empty)
*/
static void proceed_to_gate(int id) {
	int sval, i,j;
	printf("[AC]: Airplane %d requesting gate\n", id);
	
	int stop = FALSE;
	sem_wait(&gate_sem);	/* decrement gate semaphore to show its occupied */
	sem_getvalue(&gate_sem, &sval);

	/* free the taxiway to proceed to a gate */
	 for (j = 0; j < NUMBER_OF_RUNWAYS; j++) {
		for (i = 0; i < 3; i++) {
			if (taxiway_array[j][i] == id) {
				sem_post(&queue_sem[j]);	/* freeing a space in the taxiway for a particular runway */
				taxiway_array[j][i] = 0;	/* by setting value to 0 */
				stop = TRUE;
				break;
			}
		}
		if (stop == TRUE){
			break;
		}
	}

	/* occupy a gate by setting value to the id of the plane */
	for (i = 0; i < NUMBER_OF_GATES; i++) {
		if (gate_array[i] == 0) {			// if the value assigned to a runway is 0, then its free
			gate_array[i] = id;				// assign the ID of the plane to the runway which is going to be used
			break;
		}
	}
	printf("[GC]: Airplane %d assigned gate %d\n", id, i + 1);
}

/*
* The function called by a thread to proceed to the hangar
* I have set a default waiting time of 10 seconds before the airplane goes to the hangar
*/
static void proceed_to_hangar(int id) {
	int i;
	sleep(5);
	printf("[AC]: Airplane %d heading to hangar\n", id);
	// Empty the gate by setting the value of the gate in the gate_array to 0 (gate is empty now)
	for (i = 0; i < NUMBER_OF_GATES; i++) {
		if (gate_array[i] == id) {
			gate_array[i] = 0;
			sem_post(&gate_sem);	/* free the gate */
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
	proceed_to_taxiways(plane->id);
	proceed_to_gate(plane->id);
	proceed_to_hangar(plane->id);
	return NULL;
}

/*
* the main function
*/
int main(int argc, char* argv[]) {
	int i, status;
	airplanes* planes;

	runway_init(runway_array);
	gate_init(gate_array);
	taxiway_init(taxiway_array);

	// initialize semaphores
	 if (sem_init(&runway_sem, 0, NUMBER_OF_RUNWAYS) || sem_init(&gate_sem,0,NUMBER_OF_GATES)) {
        printf("Semaphores could not be initialized!\n");
        return 1;
    }

    for (i = 0; i < NUMBER_OF_RUNWAYS; i++) {
    	if (sem_init(&queue_sem[i], 0, 3)) {
    		printf("Semaphores could not be initialized!\n");
       		return 1;
    	}
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

	// destorying the semaphores
	if (sem_destroy(&runway_sem) || sem_destroy(&gate_sem)) {
        printf("Unable to destory Sempahores!\n");
        exit(1);
    }

    for (i = 0; i < NUMBER_OF_RUNWAYS; i++) {
    	sem_destroy(&queue_sem[i]);
    }

	return 0;
}
