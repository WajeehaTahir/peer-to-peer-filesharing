#include <iostream>
#include <thread>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>

#define SERVER_PORT_NO 80

using namespace std;

int joinConnection(string, int);
int makeConnection(int, string);
void getFile(int, int);
void output(int);
void input(int);

bool chat_flag = false;		//flag to pause chat with server
int wait_flag = 1;	//flag to wait for a response from user

void getFile(int connfd, int connfd_server)
{	
	cout << "Chat with peer? y/n, e for exit" << endl;
	while(wait_flag == 1);	//wait for response
	
	if(wait_flag == 2)	//if yes
	{
		thread out(output,connfd);
		thread in(input,connfd);
		in.join();	
	  	out.join();
  	}
  	
  	cout << "Chat closed, file is being downloaded" << endl;
	
	char buffer[100];
	bzero(buffer, 100);
	recv(connfd, buffer, 100, 0);
	string filename = buffer;
	ofstream file("files/" + filename);	//create empty file
	while(1)
	{
		bzero(buffer, 100);
		recv(connfd, buffer, 100, 0);
		if(!strcmp(buffer, "\n\r\n"))	//if end sequence, end
		{
			break;
		}
		file << buffer;	//write buffer to file
	}
	file.close();
	
	cout << "File download complete, closing connection" << endl;
	
	send(connfd, "e", strlen("e"), 0);	//notify seeder to end connection
  	close(connfd);
  	
  	send(connfd_server, "files\n", strlen("files\n"), 0);	//add downloaded file to server database
  	sleep(0.5);
  	send(connfd_server, (filename + "\n").c_str(), strlen((filename + "\n").c_str()), 0);
  	sleep(0.5);
  	send(connfd_server, "\n\r\n", strlen("\n\r\n"), 0);
}
int joinConnection(string ip = "127.0.0.1", int port = 80)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("Socket Creation failed\n");
		return -1;
	}

	struct sockaddr_in s_addr;

	s_addr.sin_family 	= AF_INET;
	s_addr.sin_port	= htons(port);
	inet_aton(ip.c_str(), &s_addr.sin_addr);
	
	if (connect(fd, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) {
		perror("Socket Connect failed\n");
		return -1;
	}
	
	return fd;
}
int makeConnection(int connfd_server, string filename)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("Socket Creation failed\n");
		return -1;
	}

	struct sockaddr_in addr;

	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_family	= AF_INET;
	addr.sin_port		= htons(0);	//zero chooses a free port

	if (::bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		perror("Bind failed on socket\n");
		return -1;	
	}

	int backlog = 1;
	if (listen(fd, backlog) == -1) {
		perror("Listen Failed on server: \n");
		return -1;
	}
	
	struct sockaddr_in cliaddr;
	socklen_t cliaddr_len = sizeof cliaddr;
	
	socklen_t sin_addr_len = sizeof addr;
	getsockname(fd, (struct sockaddr *)&addr, &sin_addr_len);
	
	cout << "File download request received, listening for connection on port: " << ntohs(addr.sin_port) << endl;
	string new_port = to_string(ntohs(addr.sin_port));	//port number of newly created port
	
	send(connfd_server, "port\n", strlen("port\n"), 0);
	sleep(1);
	send(connfd_server, new_port.c_str(), strlen(new_port.c_str()), 0);	//send port number to server
	
	int connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddr_len);
	if (connfd <=0 ) {
		perror("accept failed on socket: ");
	}
	cout << "Peer connected" << endl;
	cout << "Chat with peer? y/n, e for exit" << endl;
	
	while(wait_flag == 1);	//wait for response
	
	if(wait_flag == 2)	//if yes, create chat threads
	{
		thread out(output,connfd);
		thread in(input,connfd);
		
		in.join();
		out.join();
	}
  	
  	cout << "Closing chat, file is being sent\n" << endl;
  	sleep(1);
	
	send(connfd, filename.c_str(), strlen(filename.c_str()), 0);	//send filename
	sleep(1);
	
	ifstream file("files/" + filename);
	string text;
	
	while(getline(file, text))
	{
		send(connfd, (text + "\n").c_str(), strlen((text + "\n").c_str()), 0);	//send file contents
	}
	
	sleep(1);
	send(connfd, "\n\r\n", strlen("\n\r\n"), 0);	//send end sequence
	
	char buffer[1];
	bzero(buffer, 1);
	recv(connfd, buffer, 1, 0);	//wait for "e" from leecher
  	
  	close(connfd);
  	cout << "Connection closed with peer" << endl;

	return 1;
}
void input(int connfd)
{
	while(1)
	{
		char buffer[1024];
		bzero(buffer, 1024);
		recv(connfd, buffer, 1024, 0);
		string b = buffer;
		
		if(!strcmp(buffer,"e")) 	//if server ends connection
		{
			break;
		}
		else if(b.substr(0, 7) == "seeder\n")	//if server sends seeder information
		{
			string ip = b.substr(7, b.find("\t"));
			string port = b.substr(b.find("\t") + 1, strlen(b.c_str()));
			chat_flag = true;	//pause chat with server
			int fd = joinConnection(ip, stoi(port));
			thread downloader(getFile, fd, connfd);
			downloader.join();
			chat_flag = false;	//resume chat with server
		}
		else if (b.substr(0, 5) == "seed\n")	//if server forwards file request
		{
			string filename = b.substr(5, strlen(b.c_str()));
			chat_flag = true;
			thread connect(makeConnection, connfd, filename);
			connect.join();
			chat_flag = false;
		}
		else
		{
			cout << buffer << endl;
		}
	}
}
void output(int connfd)
{
	while(1)
	{
		char buffer[100];
		bzero(buffer, 100);
		cin.getline(buffer, 100);
		if(!strcmp(buffer,"y") || !strcmp(buffer,"n") )
		{
			if(!strcmp(buffer,"y"))
			{
				wait_flag = 2;	//flag to create chat
			}
			else
			{
				wait_flag = 0;
			}
			while(chat_flag);	//pause thread until chat_flag is false
			wait_flag = true;
		}
		else
		{
			send(connfd, buffer, strlen(buffer), 0);

			if(!strcmp(buffer,"e"))	//e for exit
			{
				break;
			}
		}
	}
}

int main() {

	int fd = joinConnection();
	
	if (fd > 0)
	{
		send(fd, "files\n", sizeof("files\n"), 0);
		
		DIR *dp;
		struct dirent *dirp;
		dp = opendir("./files");
		
		while((dirp = readdir(dp)) != NULL)
		{
			string filename = dirp->d_name;
			if(filename[0] != '.')
			{
				send(fd, filename.c_str(), sizeof(filename), 0);	//send filenames
			}
		}
		
		send(fd, "\n\r\n", sizeof("\n\r\n"), 0);	//send end sequence
		closedir(dp);
		
		thread out(output,fd);	//threads to chat with server
		thread in(input,fd);
		out.join();
	  	in.join();

	    	close(fd);
    	}
	return 0;
}
