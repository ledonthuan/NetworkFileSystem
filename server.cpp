#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include "FileSys.h"
using namespace std;

bool receive(string &response, int sock);

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    int port = atoi(argv[1]);

    //networking part: create the socket and accept the client connection


    int sock = socket(AF_INET, SOCK_STREAM, 0); //change this line when necessary! 

    if (sock == -1)
    {
        cerr <<" Socket failure. " << endl;
        return -1;
    }

    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 

    if (bind (sock,(sockaddr*) &servaddr, sizeof(servaddr)) == -1)
    {
        cerr << "Binding failed..." << endl;
        return -1;
    }

    cout << "port: " << port << endl;



    listen(sock, 1);



    cout << "listening.." << endl;

    int new_fd = accept(sock, NULL, 0); 

    close(sock);

    cout << "closed " << endl;





    //mount the file system
    FileSys fs;
    fs.mount(new_fd); //assume that sock is the new socket created 
                    //for a TCP connection between the client and the server.   
 
    cout << "Mounted..." << endl;
    //Here!!
    //loop: get the command from the client and invoke the file
    //system operation which returns the results or error messages back to the clinet
    //until the client closes the TCP connection.

    string response;
    string cmd;
	//string cmd1;
	string level_one;
	string level_two;
	string data; 
	string space, value, intermediate;


    int pos;
    while(true){
        if (!receive(response, new_fd))
            break;
        
        string fullMessage = response.substr(0, response.length() - 2);
        string command = fullMessage.substr(0, fullMessage.find(" "));
        
        if (command == "mkdir"){
            cmd = fullMessage.substr(5);
			cerr << "full msg: " << fullMessage << endl;
			cerr << "cmd: " << cmd << endl;
            fs.mkdir(cmd.c_str());
        }
        else if (command == "ls"){
            //cmd = fullMessage.substr(2);
            fs.ls();
        }
        else if (command == "cd"){
            cmd = fullMessage.substr(2);
            fs.cd(cmd.c_str());
        }
        else if (command == "home"){
            fs.home();
        }
        else if (command == "rmdir"){
            cmd = fullMessage.substr(5);
            fs.rmdir(cmd.c_str());
        }
        else if (command == "create"){
            cmd = fullMessage.substr(6);
            fs.create(cmd.c_str());
        }
        else if (command == "append"){
            // NEED TO IMPLEMENT
            //double check
            cmd = fullMessage.substr(7);
			// cerr << "full msg: " << fullMessage << endl; 
			// cerr << "cmd: " << cmd << endl;

			// cmd1 = cmd.substr(0, cmd.find(" "));
			// cerr << "cmd1: " << cmd1 << endl;

			// data = cmd.substr(cmd.find(" ")+1);
			// cerr << "data: " << data << endl;

            // fs.append(cmd1.c_str(), data.c_str());

			level_one = fullMessage.substr(7);
			level_two = level_one.substr(0, level_one.find(" "));
			data = level_one.substr(level_two.length() + 1);
			fs.append(level_two.c_str(), data.c_str());

        }
        else if (command == "stat"){
            cmd = fullMessage.substr(4);
            fs.stat(cmd.c_str());
        }
        else if (command == "cat"){
            cmd = fullMessage.substr(3);
            fs.cat(cmd.c_str());
        }
        else if (command == "head"){
            // NEED TO IMPLEMENT
            //double check
            space = fullMessage.substr(0, fullMessage.find("\r\n"));
			level_one = space.substr(4);
			value = level_one.substr(1, level_one.find(" "));
			intermediate = level_one.substr(value.length()+1);
			level_two = intermediate.substr(0, intermediate.find(" "));
			data = intermediate.substr(level_two.length() + 1);		   
			fs.head(level_two.c_str(), stoi(data));
        }
        else if (command == "rm"){
            cmd = fullMessage.substr(2);
            fs.rm(cmd.c_str());
        }
    }
    //close the listening socket
    close(new_fd);

    //unmout the file system
    fs.unmount();

    return 0;
}


bool receive(string &response, int sock)
{
  vector<char> msg;
  int temp = 0;
  int byteSent = 0;
  int count = 0;
  char buf;
  bool done = false;
  bool endP; false;

	while (!endP || !done)
	{
	   count = 0;

		while (count < sizeof(char))
		{
		    if ((byteSent = recv(sock, (void*)&buf, sizeof(char), 0)) == -1)
                perror("Error receiving message");

			count += byteSent;
		}

     	if (buf == '\r')
        	endP = true;

		if (buf == '\n')
			done = true;
		msg.push_back(buf);
  }
  

	response = string(msg.begin(), msg.end());

	return true;
}
