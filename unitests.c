#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	char* request = (char*)malloc(sizeof(char*)*1024);
	char* getRequests[16] = {"GET /index.html HTTP/1.0",
							"GET / HTTP/1.0 ",
							"GET /gnu/main.html HTTP/1.0",
							"GET /nofile HTTP/1.0", 
							"GET /gnu/nofile HTTP/1.0",
							"get / http/1.0",
							"GET /Index.html HTTP/1.0",
							"GET http.request HTTP/1.0",
							"GET /cantRead HTTP/1.0",
							"GET /index.html HTTP/1.1",
							"GET /gnu/ HTTP/1.1",
							"got / http/1.0 ",
							"GET /../ HTTP/1.0"
							};
	char* sourceip = argv[1];
	char* hostip = argv[2];
	char* port = argv[3];
	int i;
	for(i=0; getRequests[i] != NULL; i++)
	{
		sprintf(request, "echo -e -n \"%s\" | nc -u -s %s %s %s", 
				getRequests[i],
				sourceip,
				hostip,
				port);
		printf("requesting: %s\n",request);
		system(request);
		printf("sleeping...\n");
		sleep(5);
	}
	return 0;
}