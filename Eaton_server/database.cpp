/**
 * Database part of test application
 * It's implemented as map of following pairs:
 * <Client name> <Number of measurements>
 * Client name - C++ string (std::string)
 * Number of measurements - number of messages received from each client (int)
 * AddMeasurement - thread unsafe, handles database update
 * PrintDatabase - thread unsafe, puts database pairs to the standard output
 */

#include <map>
#include <cstdio>
#include <iostream>
#include <climits>

using namespace std;

static map<string, int> measurementDatabase;	///< Database of client messages

/* Function prototypes to be called from C code */
extern "C" void AddMeasurement(char* client_name);
extern "C" void PrintDatabase(void);

void AddMeasurement(char* client_name)
{
	string name_string(client_name);			///< Client name constructed from C-string

	/* Check if database contains item with current client name */
	auto name_count = measurementDatabase.count(name_string);

	if(name_count == 0)
	{
		measurementDatabase[name_string] = 1;
	}
	else
	{
		int val = measurementDatabase[name_string];
		/* Avoiding overflow */
		if (val < INT_MAX)
			measurementDatabase[name_string] = ++val;
	}
}

void PrintDatabase(void)
{
	/* Command to clean the screen */
	cout << "\x1B[2J\x1B[H";

	/* Print all database items */
	for(auto& x : measurementDatabase)
	{
    	cout << x.first << " sent data " << x.second << " times" << endl;
	}
}
