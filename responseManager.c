#include "responseManager.h"

/*
* Object to store properties of requested file
*/	
static struct file{
	FILE* object;
	char* path;
	char* contents;
} _file;

/*
* Store the root directory of the server
*/
char* _root;

/*
* Mount the server on "root" by simply declaring a string
*/
void mount_onroot(char* root)
{
	_root = root;
}
/*
* Format and return "timestamp" 
*/		
char* format_timestamp(char* timestamp)
{
	char* month = (char*)malloc(sizeof(char*)*100);
	int monthNum;
	
	while(timestamp[0] != ' ')
	{
		charAppend(month, timestamp[0]);
		timestamp++;
	}
	timestamp++;
	char* monthAbrv = (char*)malloc(sizeof(char*)*100);
	monthNum = atoi(month);
	switch(monthNum)
	{
		case 1:
			monthAbrv = monthSet.jan;
			break;
		case 2:
			monthAbrv = monthSet.feb;
			break;
		case 3:
			monthAbrv = monthSet.mar;
			break;
		case 4:
			monthAbrv = monthSet.apr;
			break;
		case 5:
			monthAbrv = monthSet.may;
			break;
		case 6:
			monthAbrv = monthSet.jun;
			break;
		case 7:
			monthAbrv = monthSet.jul;
			break;
		case 8:
			monthAbrv = monthSet.aug;
			break;
		case 9:
			monthAbrv = monthSet.sep;
			break;
		case 10:
			monthAbrv = monthSet.oct;
			break;
		case 11:
			monthAbrv = monthSet.nov;
			break;
		case 12:
			monthAbrv = monthSet.dec;
			break;
		
	}
	timestamp = strAppend(monthAbrv, timestamp);
	return timestamp;
}
/*
* Parse the datagram packet and return the according response
*/
char* get_responseline(char datagram[])
{
	char delims[3] = {' ', '\n'};
	char* tok;
	char** requestLine = (char**)malloc(sizeof(requestLine)*1024);
	char* responseLine;
	// Tokenize the datagram packet
	tok = strtok(datagram, delims);
	int i=0;
	while(tok != NULL)
	{
		requestLine[i] = malloc(sizeof(char*)*strlen(tok));
		strcpy(requestLine[i], tok);
		tok = strtok(NULL, " ");
		i++;
	}

	// Determine filepath to open
	char* fileArg = requestLine[1];	// the argument in the requestLine sent by the client  eg: /index.html
	char* filePath = (char*)malloc(sizeof(char*)*1024);	// the entire path to the file starting from the root eg: /A1/www/index.html
	
	if(strncmp(fileArg, "/\0", 2)==0)
		fileArg = "/index.html";
	strcpy(filePath, _root);
	strcpy(&filePath[strlen(_root)], fileArg);

	// Make sure user didn't request a directory
	if(isDir(filePath))
		responseLine = responseSet._404;
	else
	{
		// Attempt to open the file and attach attributes to _file property
		long length;
		FILE* f = fopen(filePath, "r");
		if (f == NULL)
		{
			responseLine = responseSet._404;
		}
		else
		{
			_file.object = f;
			fseek (f, 0, SEEK_END);
			length = ftell (f);
			fseek (f, 0, SEEK_SET);
			_file.contents = malloc (length);
			if (_file.contents)
			{
				fread (_file.contents, 1, length, f);
			}
			_file.path = filePath;
			fclose (f);
			responseLine = responseSet._200;
		}
	}
	return responseLine;
}
/*
* Check if "filePath" is a directory instead of a file
*/
bool isDir(char* filePath)
{
	struct stat st;
	
	DIR* dir = opendir(filePath);
	if(dir)
	{
		struct dirent* dent = readdir(dir);
		lstat(dent->d_name, &st);
		if(S_ISDIR(st.st_mode))
		{
			return true;
		}
		return false;
	}
	// can't prove that it's a dir, can't prove it's a file either
	// if user requested non-existing dir, then fopen will catch that later
	return false; 
}
/*
* Check if "requestLine" is a bad request
* 		Uses a regular expression library see --> https://github.com/cesanta/slre
*/
bool isBadRequest(char* requestLine)
{
	bool isBad = false;
		
	struct slre_cap caps[4];

	if (slre_match("^\\s*(\\S+)\\s+(\\S+)\\s+HTTP/(\\d)\\.(\\d)",
				   requestLine, strlen(requestLine), caps, 4, 0) <= 0) {
		isBad = true;
	}else{
		//printf("Method: [%.*s], URI: [%.*s]\n",caps[0].len, caps[0].ptr,caps[1].len, caps[1].ptr);
	}
	
	return isBad;
}

/*
* Get the path associated with the _file object
*/
char* get_filePath()
{
	return _file.path;
}
/*
* Get the contents associated with the _file object
*/
char* get_fileContents()
{
	return _file.contents;
}
/*
* Reset the _file object
*/
void free_file()
{
	_file.object = NULL;
	_file.path = NULL;
	_file.contents = NULL;
}
/*
* Utility: Append c to s
*/
char* charAppend(char* s, char c)
{
	int len = strlen(s);
	s[len] = c;
	return s;
}
/*
* Utility: Append str2 to str1
*/
char* strAppend(char* str1, char* str2)
{
	char* str3 = (char*)malloc(sizeof(char*)*(strlen(str1)+strlen(str2)));
	strcpy(str3, str1);
	strcat(str3, str2);
	return str3;
}