
/*
 * CS61C Summer 2016
 * Name: Yuki Mizuno, Cat Ashley
 * Login: cs61c-al, cs61c-co
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "flights.h"
#include "timeHM.h"

struct flightSys {
	struct airport* headAir; //pointer to its first airport	
};

struct airport {
	struct flight *headF; //pointer to the airport's first flight in its schedule
	struct airport *nextAir; //pointer to the next airport node
	char *airportName; //pointer to the airport's name, stored as a char array

};

struct flight {
	struct airport *dest; //pointer to the flight's destination airport
	timeHM_t *depart; //pointer to the flight departure time
	timeHM_t *arriv; //pointer to the flight arrival time
	int costF; //flight cost
	struct flight *nextF; //pointer to the  next flight node
};

/*
   This should be called if memory allocation failed.
 */
static void allocation_failed() {
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
}


/*
   Creates and initializes a flight system, which stores the flight schedules of several airports.
   Returns a pointer to the system created.
 */
flightSys_t* createSystem() {
	flightSys_t *ptr; //new pointer to a flightSys
	ptr = (flightSys_t *) malloc (sizeof (flightSys_t)); //make space for the flightSys
	if (ptr == NULL) { //run allocation_failed() if memory allocation failed
		allocation_failed();
	}
	ptr->headAir = NULL; //this new flightSys has no airports. Set its first airport to NULL
	return ptr; //finally, return the pointer to the new flightSys
}



/*
  Frees all memory associated with this flight
*/
void deleteFlight(struct flight* f) {
	if (f) { //if f is a defined flight pointer,
		 //free its attributes before finally
		 //f itself
		free(f->dest);
		free(f->depart);
		free(f->arriv);	
		deleteFlight(f->nextF); //recursive call for this linked list
		free(f);
	}
}

/*
  Frees all memory associated with this airport
*/
void deleteAirport(airport_t* a) {
	if (a) { //if a is a defined airport pointer,
		 //free its attributes before finally
		 //a itself
		deleteFlight(a->headF);
		free(a->airportName);
		deleteAirport(a->nextAir); //recursive call for this linked list
		free(a);
	}
}

/*
   Frees all memory associated with this system; that's all memory you dynamically allocated in your code.
 */
void deleteSystem(flightSys_t* s) {
	if (s) { //if s is a defined system pointer,
		 //free its attributes before finally
		 //s itself
		
		if (s->headAir == NULL) {
			free(s->headAir);
			free(s);
		} else {
			deleteAirport(s->headAir);
			free(s);
		}		
	}
}



/*
   Adds a airport with the given name to the system. You must copy the string and store it.
   Do not store "name" (the pointer) as the contents it point to may change.
 */
void addAirport(flightSys_t* s, char* name) {
	if (s == NULL) {
		fprintf(stderr, "addAirport: passed a NULL flightSys.\n");
		abort();
	}
	airport_t* thisAir; //make new airport and char pointers
	char *thisName;

	thisAir = (airport_t *) malloc (sizeof (airport_t)); //give them space
	if (!thisAir) {
		allocation_failed();
	}
	thisName = (char *) malloc (strlen(name) + 1);
	if (!thisName) {
		allocation_failed();
	}

	strcpy(thisName, name); //copy and store the name string

	thisAir->airportName = thisName; //set the attributes for the new airport
	thisAir->headF = NULL;
	thisAir->nextAir = NULL;

	if (s->headAir == NULL){ //make the new airport s's head airport if there is none
		s->headAir = thisAir;
	} else { //otherwise add it to the end of the linked list of airports
		airport_t* aptr;
		aptr = s->headAir;
		while (aptr != NULL && aptr->nextAir != NULL)
			aptr = aptr->nextAir;
		aptr->nextAir = thisAir;
	}
}


/*
   Returns a pointer to the airport with the given name.
   If the airport doesn't exist, return NULL.
 */
airport_t* getAirport(flightSys_t* s, char* name) {
	if (s == NULL) { //check that the arguments aren't empty
		return NULL;
	} else if (name == NULL) {
		return NULL;
	}
	airport_t *thisAir = s->headAir; //create a new pointer to an airport, pointing at the first airport in s
	char* airName;

	if (thisAir != NULL) { //check that s has at least one airport
		
		airName = (char *) malloc (strlen(thisAir->airportName)*(sizeof (char))+1); 
		
		//allocate space for the airport pointer
		
		if (airName == NULL) { //check for successful memory allocation
			allocation_failed();
		}

		strcpy(airName, thisAir->airportName);
		if (strcmp(airName, name) == 0) {
			//check that the first airport in s has the name 'name'
			free(airName);
			return thisAir; //return this airport if it's a match
		}
	} else { //return NULL if s has no airports
		return NULL;
	}

	free(airName);
	//loop through the rest of the airports to find one with the name 'name'
	while ((thisAir != NULL) && (thisAir->nextAir != NULL)) {
		thisAir = thisAir->nextAir;
		char *newAN;
		newAN = (char *) malloc (strlen(thisAir->airportName)*(sizeof(char))+1);
		if (!newAN) {
			allocation_failed();
		}
		strcpy(newAN, thisAir->airportName);
		if (strcmp(newAN, name) == 0) {
			free(newAN);
			return thisAir;
		}
	}
	return NULL; 
	//indicates an airport with that name could not be found
}


/*
   Print each airport name in the order they were added through addAirport, one on each line.
   Make sure to end with a new line. You should compare your output with the correct output
   in flights.out to make sure your formatting is correct.
 */
void printAirports(flightSys_t* s) {
	if (s) { //check s is valid
		airport_t *thisAir = s->headAir;
		//check for airports in the system
		if (s->headAir == NULL)
			return;
		//if there are, loop through the linked list
		//and print the name of each airport
		while (thisAir != NULL) {
			fprintf(stdout, "%s\n", thisAir->airportName);
			thisAir = thisAir->nextAir;	
		}
	}	
}


/*
   Adds a flight to src's schedule, stating a flight will leave to dst at departure time and arrive at arrival time.
 */
void addFlight(airport_t* src, airport_t* dst, timeHM_t* departure, timeHM_t* arrival, int cost) {	
	//make pointers for the new flight and its attributes
	struct flight *thisFl; 
	airport_t *aptr;
	timeHM_t *dt, *at;

	//allocate appropriate amount of space for each pointer
	aptr = (airport_t *) malloc (sizeof (airport_t));
	if (!aptr) {
		allocation_failed();
	}

	dt = (timeHM_t *) malloc (sizeof(timeHM_t));
	if (!dt) {
		allocation_failed();
	}

	at = (timeHM_t *) malloc (sizeof(timeHM_t));
	if (!at) {
		allocation_failed();
	}

	thisFl = (struct flight *) malloc (sizeof (struct flight));

	if (!thisFl) {
		allocation_failed();
	}


	//dereference each pointer to the arguments
	*aptr = *dst;
	*dt = *departure;
	*at = *arrival;

	//assign the corresponding values to each of the new flight's
	//attributes
	thisFl->dest = aptr;
	thisFl->depart = dt;
	thisFl->arriv = at;
	thisFl->costF = cost;
	thisFl->nextF = NULL;

	//make the new flight the head flight
	//if there are no flights in the source airport
	if (src->headF == NULL) {
		src->headF = thisFl;
	} else {
		//otherwise, add the new flight to the end
		//of the linked list of flights
		struct flight *fptr;
		fptr = src->headF;
		while (fptr != NULL && fptr->nextF != NULL) {
			fptr = fptr->nextF;
		}
		fptr->nextF = thisFl;
	}	
}


/*
   Prints the schedule of flights of the given airport.

   Prints the airport name on the first line, then prints a schedule entry on each 
   line that follows, with the format: "destination_name departure_time arrival_time $cost_of_flight".

   You should use printTime (look in timeHM.h) to print times, and the order should be the same as 
   the order they were added in through addFlight. Make sure to end with a new line.
   You should compare your output with the correct output in flights.out to make sure your formatting is correct.
 */
void printSchedule(airport_t* s) {
	if (s == NULL) {
		return;
 	}
	//create pointers
	struct flight *fptr;
	airport_t *dst;

	//initialize the new flight pointer
	//to the head flight of s
	fptr = s->headF;

	//loop through all flights in s,
	//print relevant info
	fprintf(stdout, "%s\n", s->airportName);
	while (fptr != NULL) {
		dst = fptr->dest;
		fprintf(stdout, "%s ", dst->airportName);
		printTime(fptr->depart);
		fprintf(stdout, " ");
		printTime(fptr->arriv);
		fprintf(stdout, " $%d\n", fptr->costF);
		fptr = fptr->nextF;
	}
}


/*
   Given a src and dst airport, and the time now, finds the next flight to take based on the following rules:
   1) Finds the cheapest flight from src to dst that departs after now.
   2) If there are multiple cheapest flights, take the one that arrives the earliest.

   If a flight is found, you should store the flight's departure time, arrival time, and cost in departure, arrival, 
   and cost params and return true. Otherwise, return false. 

   Please use the function isAfter() from time.h when comparing two timeHM_t objects.
 */
bool getNextFlight(airport_t* src, airport_t* dst, timeHM_t* now, timeHM_t* departure, timeHM_t* arrival, int* cost) {
	struct flight *curr, *final; //create flight pointers
	final = NULL;

	if (src == NULL) {
		return false;
	}
	
	//initialize the CURRent flight pointer to the first
	//flight in airport src
	curr = src->headF;
	if (curr == NULL) {
		return false;
	}
	//loop through all flights, checking for the desired conditions...
	while (curr != NULL) {
		//check for the desired destination and that the flight leaves after now
		if (!strcmp(curr->dest->airportName, dst->airportName) && isAfter(curr->depart, now)) {
			if (final == NULL) {
				final = curr;
			} else if (final->costF > curr->costF) {
				//make the FINAL flight pointer
				//point to the cheaper flight
				final = curr;
			} else if (final->costF == curr->costF) {
				//make the FINAL flight pointer
				//point to the cheapest flight that
				//also arrives at the destination earlier
				if (isAfter(final->arriv, curr->arriv)) {
					final = curr;
				}
			}
		}
		curr = curr->nextF;
	}
	if (final == NULL) {
		return false;
	}
	
	//store the desired values and return true
    	timeHM_t *newDeparture = (timeHM_t *) malloc (sizeof (timeHM_t));
	if (!newDeparture) {
		allocation_failed();
	}

	timeHM_t *newArrival = (timeHM_t *) malloc (sizeof (timeHM_t));
	if (!newArrival) {
		allocation_failed();
	}

	int *newCost = (int *) malloc (sizeof (int));
	if (!newCost) {
		allocation_failed();
	}

	*newDeparture = *(final->depart);
	*newArrival = *(final->arriv);
	*newCost = final->costF;

	*departure = *newDeparture;
	*arrival = *newArrival;
	*cost = *newCost;

	//free the malloc'd space
	free(newDeparture);
	free(newArrival);
	free(newCost);

	return true;
}
