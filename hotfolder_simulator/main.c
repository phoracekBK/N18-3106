#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h> 
#include <dirent.h>

#include <windows.h>

/********************* constants **********************/

#define HSIM_OUTPUT_FILE_NAME "SLOPSA_OUT.txt"

enum hsim_status
{
	HSIM_SUCCESS = 0,
	HSIM_BAD_INPUT_PARAMETERS,
	HSIM_INPUT_PARAMETERS_SUCCESS,
	HSIM_MOVING_FILE,
	HSIM_READING_HOTFOLDER,

	HSIM_STATUS_NUM
};

const char * status_str[] = {"Reading hotfolder", 
				"Bad input parameters", 
				"Input parameters readed",
				"Moving file",
				"Reading hotfolder"};


/*************** local variables *************************/
char * hotfolder_address = NULL;
char * file_copy_destination_address = NULL;
uint32_t reading_rate = 0;
uint8_t status = 255;
uint8_t status_trigger = 255;
boolean cycle_run_control = true;
boolean error = false;
DIR * dir_reference = NULL;




/**************** function declarations ********************/
void setup(int argv, char ** argc);
void print_current_status();
boolean is_string_number(char * string);
void trigger_runtime();
void runtime_control();
void probe_hotfolder_and_move_file();
uint8_t move_file(char * source_address, char * destination_address, char * file_name);
void read_hotfolder();
void open_directory_if_not_open();
void close_directory_if_open();
void run();


/**************** function definitions *********************/

void setup(int argv, char ** argc)
{
	if((argv > 3) && (strlen(argc[1]) > 0) && (strlen(argc[2]) > 0) && is_string_number(argc[3]))
	{
		hotfolder_address = (char*) malloc(sizeof(argc[1])+1);
		strcpy(hotfolder_address, argc[1]);


		file_copy_destination_address = (char*) malloc(sizeof(argc[2])+1);
		strcpy(file_copy_destination_address, argc[2]);
		
		reading_rate = atoi(argc[3]);	
		status = HSIM_INPUT_PARAMETERS_SUCCESS;
	}
	else
	{
		status = HSIM_BAD_INPUT_PARAMETERS;
	}
}

boolean is_string_number(char * string)
{
	for(int i = 0; i < strlen(string); i++)
	{
		if(isdigit(string[i]) == 0)
			return false;
	}

	return true;
}

void print_current_status()
{
	if(status != status_trigger)
	{
		if(status < HSIM_STATUS_NUM)
			printf("%d - %s\n", status, status_str[status]);
		else
			printf("%d - Neznámí vnitřní stav\n", status);
	}
}

void trigger_runtime()
{
	status_trigger = status;
}


void runtime_control()
{
	if(status == HSIM_BAD_INPUT_PARAMETERS)
	{
		cycle_run_control = false;
	}
	else
	{
		cycle_run_control = true;
	}
}

uint8_t move_file(char * source_address, char * destination_address, char * file_name)
{
	if((source_address != NULL) && (destination_address != NULL) && (file_name != NULL))
	{
		char dest_addr[255];
		char src_addr[255];

		sprintf(dest_addr, "%s/%s", destination_address, file_name);
		sprintf(src_addr, "%s/%s", source_address, file_name);
														
		return rename(src_addr, dest_addr);
																		}
	else
	{
		return 0;
	}
}

void probe_hotfolder_and_move_file()
{
	struct dirent * dir_cont;

	if((dir_cont = readdir(dir_reference)) != NULL)
	{
		if((dir_cont->d_type != DT_DIR) && (strcmp(dir_cont->d_name, HSIM_OUTPUT_FILE_NAME) == 0))
		{
			move_file(hotfolder_address, file_copy_destination_address, HSIM_OUTPUT_FILE_NAME);
			status = HSIM_MOVING_FILE;
		}
		else
		{
			status = HSIM_READING_HOTFOLDER;
		}
	}
	else
	{
		close_directory_if_open();
	}	
}

void open_directory_if_not_open()
{
	if(dir_reference == NULL)
		 dir_reference = opendir(hotfolder_address);
}


void close_directory_if_open()
{
	if(dir_reference != NULL)
	{
		closedir(dir_reference);
		dir_reference = NULL;
	}
}

void read_hotfolder()
{
	open_directory_if_not_open();
	
	if(dir_reference != NULL)
		probe_hotfolder_and_move_file();
}

void run()
{
	read_hotfolder();

	print_current_status();
	trigger_runtime();

	runtime_control();
}


int main(int argv, char ** argc)
{
	setup(argv, argc);

	while(cycle_run_control)
	{
		run();
		Sleep(reading_rate);
	}

	return status;
}



