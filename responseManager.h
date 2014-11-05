#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "slre.h" // regular expression library see --> https://github.com/cesanta/slre

//============ Constants =============
static const struct monthSet{
	char* jan;
	char* feb;
	char* mar;
	char* apr;
	char* may;
	char* jun;
	char* jul;
	char* aug;
	char* sep;
	char* oct;
	char* nov;
	char* dec;
} monthSet = {"Jan", "Feb", "Mar", "Apr", 
			"May", "Jun", "Jul", "Aug", 
			"Sep", "Oct", "Nov", "Dec"};
			
static const struct responseSet{
	char* _200;
	char* _400;
	char*_404;
} responseSet = {"HTTP /1.0 200 OK", 
				"HTTP /1.0 400 Bad Request", 
				"HTTP /1.0 404 Not Found"};
//======================================

				
void mount_onroot(char*);
char* format_timestamp(char*);
char* get_responseline(char[]);
bool isDir(char*);
bool isBadRequest(char*);
bool tooManyArgs(char**);
bool notEnoughArgs(char**);
char* get_filePath();
char* get_fileContents();
void free_file();
char* charAppend(char*, char);
char* strAppend(char*, char*);

