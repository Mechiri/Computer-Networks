/*
 *  Here is the starting point for your Assignment 02 definitions. Add the 
 *  appropriate comment header as defined in the code formatting guidelines
 */

#include <stdio.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)


struct arguments
{
	int* client_fd;
	struct sockaddr_in* client_addr;
};

//function check at the client side input and makes it end connection if it is "goodbye" and "exit"
void check_input(char* write_buffer,int* connection)
{
	if(strcmp(write_buffer, "goodbye") == 0)
	{
   		*connection = 0;
	}
	if(strcmp(write_buffer, "exit") == 0)
	{
   		*connection = 0;
	}
}

//function check at the server side recived input and makes it reply according to assignment
void set_buffer(char* buffer, int* connection)
{
	int buffer_size = sizeof(buffer);
	if(strcmp(buffer, "hello") == 0)
	{
		memset(buffer, 0, buffer_size);
   		strcpy(buffer, "world");
	}
	if(strcmp(buffer, "goodbye") == 0)
	{
		memset(buffer, 0, buffer_size);
   		strcpy(buffer, "farewell");
   		*connection = 0;
	}
	if(strcmp(buffer, "exit") == 0)
	{
		memset(buffer, 0, buffer_size);
   		strcpy(buffer, "ok");
   		*connection = 0;
	}
}

/* TCP STARTS HERE*/

//server thread handler
void* manage_connections(void* args)
{

	struct arguments *arg = args;
	char buffer[2048];
	int sockid= *(arg->client_fd);
	int connection = 1;
	int exit_status = 0;
	int loop = 50;

	while( connection)
	{
		memset(buffer, 0, sizeof(buffer));
		//read from client fd
		if ( read(sockid, buffer, 2000) < 0 ) 
	  		handle_error("Reading client Error: ");

	  	printf("got message from ('%s', %d)\n", inet_ntoa(arg->client_addr->sin_addr), ntohs(arg->client_addr->sin_port));
    	//printf("%s\n", buffer);
		
		//check client input
		if(strcmp("exit", buffer) == 0)
			exit_status = 1;
		set_buffer(buffer, &connection);

		//write to client fd
		if (write(sockid, buffer, strlen(buffer)) < 0)
	  		perror("writing client Error: ");
	  	if(exit_status)
	  		exit(0);
	  	if(loop == 0)
	  	{
	  		pthread_exit(0);
	  	}
	  	--loop;
	}
	//close socket	
	close(sockid);
	pthread_exit(0);
	return NULL;
}


void start_server(long port)
{
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;
	socklen_t server_addr_len, client_addr_len;
	pthread_t thd;
	int clients_count = 0;
	struct arguments args;
	int connection = 1;


	//Server Address Information
	//IPV4
	server_addr.sin_family = AF_INET;
	//network byte order
  	//check "cat proc/sys/net/ipv4/ip_local_port_range"
  	server_addr.sin_port = htons(port);
	//INADDR_ANY is specified in the bind call, the socket will be bound to all local interfaces
  	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  	//size of an server address
  	server_addr_len = sizeof(struct sockaddr_in);
  	//get client address size for client accept
  	client_addr_len = sizeof(struct sockaddr_in);

 	//create server file descriptor
	server_fd = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP-3 argument
	if(server_fd == -1)
            fprintf(stderr, "socket err: %d\n", server_fd);
 

    //binding the socket file descriptor and the server interface
    //or
    //assigning a name to a sokcet
    if (bind(server_fd, (struct sockaddr *)&server_addr, server_addr_len) != 0) 
    	handle_error("Bind Error: ");


  	//the maximum length to which the queue of pending connections for sockfd may grow ept to 11
  	if (listen(server_fd, 5000) < 0) 
    	handle_error("Listen Error: ");


  	//accept read write
  	while(connection)
  	{
  		//accept incoming client request and assign to client descriptor 
  		if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) 
      		handle_error("Accept Error: ");
      	else
      	{
      		printf("connection %d from ('%s', %d)\n",clients_count++, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    		args.client_fd = &client_fd;
    		args.client_addr = &client_addr;

    		pthread_create(&thd, NULL, manage_connections, (void*)&args);
    		pthread_detach(thd);
      	}
    	
  	}
  	//close server descriptor
  	close(server_fd);
}


void start_client(char* server, long port)
{

	struct sockaddr_in server_addr;
	int server_fd;
	socklen_t server_addr_len;
	int connection=1;
	char write_buffer[2048],read_buffer[2048];

	//Server Address Information
	//IPV4 or IPv6
	server_addr.sin_family = AF_INET;
	//network byte order
  	//check "cat proc/sys/net/ipv4/ip_local_port_range"
  	server_addr.sin_port = htons(port);
  	//server address
  	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//size of an server address
  	server_addr_len = sizeof(struct sockaddr_in);

  	//create server file descriptor
	server_fd = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP-3 argument
	if(server_fd == -1)
            handle_error("Socket creation error");


    //connect with the server
    if (connect(server_fd, (struct sockaddr *)&server_addr, server_addr_len) < 0) 
    	handle_error("Not Connectiong to the server Error...");


  	while(connection)
  	{
  		memset(write_buffer, 0, sizeof(write_buffer));
  		scanf("%s", write_buffer);

  		//send message to the server
	  	if (write(server_fd, write_buffer, strlen(write_buffer)) < 0) 
	    	handle_error("Cannot able to write...");

	    //check input data
	  	check_input(write_buffer, &connection);

	  	memset(read_buffer, 0, sizeof(read_buffer));
	  	//receive message from server
	  	if (read(server_fd, read_buffer, sizeof(read_buffer)) < 0) 
	    	handle_error("Cannot able to read...");

	  	printf("%s\n", read_buffer);
	  	
  	}
  	close(server_fd);
}
/* Add function definitions */

/* UDP STARTS HERE*/

struct arguments_udp
{
	int* server_fd;
	struct sockaddr_in* client_addr;
	char buffer[2048];
};

void* manage_connections_udp(void* args)
{
	struct arguments_udp *arg = args;
	char buffer[2048];
	int sockid= *(arg->server_fd);
	int connection = 1,recvlen, exit_status=0;
	socklen_t client_addr_len = sizeof(struct sockaddr_in);

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, arg->buffer);
	

	if(strcmp("exit", buffer) == 0)
		exit_status = 1;
	//check client input information
	set_buffer(buffer, &connection);
	recvlen = strlen(buffer);

	//send information to client
	if (sendto(sockid, buffer, recvlen, 0, (struct sockaddr*) (arg->client_addr), client_addr_len) == -1)
		handle_error("sendto(client udp)");

	if(exit_status)
		exit(0);
	pthread_exit(0);
	return NULL;
}

void start_server_udp(long port)
{
	struct sockaddr_in server_addr, client_addr;
	int server_fd; 
	socklen_t server_addr_len, client_addr_len;
	int connection = 1,recvlen;// clients_count=0;
	char buffer[2048];
	pthread_t thd;
	struct arguments_udp args;

	//Server Address Information
	//IPV4 
	server_addr.sin_family = AF_INET;
	//network byte order
  	//check "cat proc/sys/net/ipv4/ip_local_port_range"
  	server_addr.sin_port = htons(port);
	//INADDR_ANY is specified in the bind call, the socket will be bound to all local interfaces
  	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;
  	//size of an server address
  	server_addr_len = sizeof(struct sockaddr_in);
  	client_addr_len = sizeof(struct sockaddr_in);

 	//create server file descriptor
	server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //UDP
	if(server_fd == -1)
            handle_error("socket creation error: ");


    //binding the socket file descriptor and the server interface
    //or
    //assigning a name to a sokcet
    if (bind(server_fd, (struct sockaddr *)&server_addr, server_addr_len) != 0) 
    	handle_error("Bind Error: ");


  	//receive and send
  	while(6)
  	{
				
		memset(buffer, 0, sizeof(buffer));
		//receive information from client
		if ((recvlen = recvfrom(server_fd, buffer, 2048, 0, (struct sockaddr *) &client_addr, &client_addr_len)) == -1)
			handle_error("recvfrom Error: ");
		else
		{
			//printf("connection %d from ('%s', %d)\n",clients_count++, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	    	printf("got message from ('%s', %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	    	//printf("%s\n", buffer);
	    	
	    	args.server_fd = &server_fd;
	    	args.client_addr = &client_addr;
	    	memset(args.buffer, 0, sizeof(buffer));
			strcpy(args.buffer, buffer);

	    	pthread_create(&thd, NULL, manage_connections_udp, (void*)&args);
	    	pthread_detach(thd);
		}
  	}
  	//close server descriptor
  	close(server_fd);

}


void start_client_udp(char* server, long port)
{

	struct sockaddr_in server_addr;
	int server_fd;
	socklen_t server_addr_len;
	int connection=1;

	char write_buffer[2048],read_buffer[2048];

	//Server Address Information
	//IPV4
	server_addr.sin_family = AF_INET;
	//network byte order
  	//check "cat proc/sys/net/ipv4/ip_local_port_range"
  	server_addr.sin_port = htons(port);
  	//server address
  	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  	
	//size of an server address
  	server_addr_len = sizeof(struct sockaddr_in);

  	//create server file descriptor
	server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //UDP
	if(server_fd == -1)
            handle_error("socket err:");


  	while(connection)
  	{
  		memset(write_buffer, 0, sizeof(write_buffer));
  		scanf("%s", write_buffer);

  		//send message to the server
	  	if (sendto(server_fd, write_buffer, strlen(write_buffer), 0 , (struct sockaddr *) &server_addr, server_addr_len) < 0) 
	    	handle_error("Cannot able to write...");

	    //check input information
	  	check_input(write_buffer, &connection);
	  	memset(read_buffer, 0, sizeof(read_buffer));
	  	if (recvfrom(server_fd, read_buffer, 2048,0 , (struct sockaddr *) &server_addr, &server_addr_len) < 0) 
	    	handle_error("Cannot able to read...");

	  	printf("%s\n", read_buffer);
	  	
  	}
  	//close socket
  	close(server_fd);
}