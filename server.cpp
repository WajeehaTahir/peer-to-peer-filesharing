#include <iostream>
#include <thread>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <list>
#include <map>

#define SERVER_PORT_NO 80

using namespace std;

struct client
{
	string ip;
	int port;
	int serial;
	int fd;
	client(string i, int p, int s, int f)
	{
		ip = i;
		port = p;
		serial = s;
		fd = f;
	}
	bool operator ==(const client& obj) const	//overloading operators for list operations
	{
		return obj.serial == serial;
	}
	bool operator !=(const client& obj) const
	{
		return obj.serial != serial;
	}
};

list <client> clients;
multimap <string, int> files;	//filenames as key, serial numbers as values

void input(client obj)
{
	int connfd = obj.fd;
	while(1)
	{
		char buffer[100];
		bzero(buffer, 100);
		recv(connfd, buffer, 100, 0);
		
		if(!strcmp(buffer, "e")) 
		{	
			send(connfd, "e", strlen("e"), 0);
			clients.remove(obj);	//remove client from list
			
			for(multimap<string, int>::iterator i = files.begin(); i!= files.end();)
			{
				if(i->second == obj.serial)
				{
				 	i = files.erase(i);	//remove files for that client
				}
				else
				{
					++i;
				}
			}
			
			string message = "Client " + to_string(obj.serial) + " has left";
			cout << endl << message << endl;
			cout << "Clients currently connected: " << endl;
			if(clients.size() > 0)
			{
				for (const auto&c:clients)
				{
					cout << to_string(c.serial) + "- IP: " + c.ip + ", Port: "+ to_string(c.port) << endl;
				}
			}
			else
			{
				cout << "None" << endl;
			}
			for (const auto&c:clients)
			{
				send(c.fd, message.c_str(), strlen(message.c_str()), 0);	//notify other clients
			}
			close(connfd);
			break;
		}
		else if(!strcmp(buffer, "files\n"))
		{
			cout << "Files: " << endl;
			while(1)
			{
				bzero(buffer, 100);
				recv(connfd, buffer, 100, 0);
				if (!strcmp(buffer, "\n\r\n"))	//if end sequence, break
				{
					break;
				}
				files.insert(pair<string, int>(buffer, obj.serial));	//store filenames with serial number
				cout << buffer << endl;
			}
			cout << endl;
		}
		else if (!strcmp(buffer, "get"))
		{
			for (const auto&f:files)
			{
				if(f.second != obj.serial)
				{
					string message = f.first + " at client " + to_string(f.second) + "\n";
					send(connfd, message.c_str(), strlen(message.c_str()), 0);	//send filenames and client serial
				}
			}
			send(connfd, "\nEnter \"none\" to go back", strlen("\nEnter \"none\" to go back"), 0);
			bzero(buffer, 100);
			recv(connfd, buffer, 100, 0);
			string filename = buffer;
			
			int serial = -1;
			for(auto i = files.begin(); i!= files.end(); i++)
			{
				if(i->first == buffer)
				{
					if(i->second != obj.serial)
					{
						serial = i->second;	//get serial number of client with that file (seeder)
					}
				}
			}
			if (serial > 0)	//if filename was not found, serial will be -1
			{
				string port;
				string ip;
				int connfd_seeder;
				
				for(auto i = clients.begin(); i!= clients.end(); i++)
				{
					if (i->serial == serial)
					{
						ip = i->ip;	//get ip address and fd of seeder
						connfd_seeder = i->fd;
					}
				}
				
				send(connfd_seeder, ("seed\n" + filename).c_str(), strlen(("seed\n" + filename).c_str()), 0);	//notify seeder to create connection
				
				bzero(buffer, 100);
				recv(connfd_seeder, buffer, 100, 0);
				
				if (!strcmp(buffer, "port\n"))
				{
					recv(connfd_seeder, buffer, 100, 0);
				}
				
				port = buffer;
				send(connfd, ("seeder\n" + ip + "\t" + port).c_str(), strlen(("seeder\n" + ip + "\t" + port).c_str()), 0); //send port number and ip address of seeder to leecher
			}
			
		}
		else if(!strcmp(buffer, "port\n"))	
		{
			sleep(3);	//pause chat for 3 seconds
		}
	}
}

int main() {
	
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("Socket Creation failed\n");
		return -1;
	}

	struct sockaddr_in addr;

	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_family	= AF_INET;
	addr.sin_port		= htons(SERVER_PORT_NO);

	if (::bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		perror("Bind failed on socket\n");
		return -1;	
	}

	int backlog = 1;
	if (listen(fd, backlog) == -1) {
		perror("Listen Failed on server: \n");
		return -1;
	}

	int count = 0;
	int connfd;
	
	struct sockaddr_in cliaddr;
	socklen_t cliaddr_len = sizeof cliaddr;
	
	while(1)
	{
		connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddr_len);
		if (connfd <=0 ) {
			perror("accept failed on socket: ");
		}
		
		count++;
		client obj(string(inet_ntoa(cliaddr.sin_addr)), int(ntohs(cliaddr.sin_port)), count, connfd);
		clients.push_back(obj);	//push client object to list
		cout << count << "- IP: " << obj.ip;
		cout << ", Port: " << obj.port << endl; 
		
		string buffer = "Client " + to_string(obj.serial) + " with IP " + string(obj.ip) + " at port " + to_string(obj.port) + " has joined";
		for (const auto&c:clients)
		{
			if(c.serial == obj.serial)
			{
				send(obj.fd, "Welcome to the server", strlen("Welcome to the server"), 0);
			} 
			else
			{
				send(c.fd, buffer.c_str(), strlen(buffer.c_str()), 0);	//notify existing clients
			}
		}
		
		send(connfd, "\nUsers already connected:", strlen("\nUsers already connected:"), 0);
		if(count > 1)
		{
			for (const auto&c:clients)
			{
				if(c.serial != obj.serial)
				{
					string buff = "\n" + to_string(c.serial) + "- " + c.ip + ", "+ to_string(c.port);
					send(connfd, buff.c_str(), strlen(buff.c_str()), 0);
				}
			}
		}
		else	//if there are no other users
		{
			send(connfd, "\nNone", strlen("\nNone"), 0);
		}
		thread in(input,obj);
		in.detach();
	}
	return 0;
}
