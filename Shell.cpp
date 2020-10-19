// CPSC 3500: Shell
// Implements a basic shell (command line interface) for the file system

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <stdio.h> 
#include <vector>
using namespace std;

#include "Shell.h"
//#include "FileSys.h"

#define MAXBUFLEN 100



static const string PROMPT_STRING = "NFS> ";	// shell prompt

// Mount the network file system with server name and port number in the format of server:port
void Shell::mountNFS(string fs_loc) {
  //
	//create the socket cs_sock and connect it to the server and port specified in fs_loc
	//if all the above operations are completed successfully, set is_mounted to true 

  // int fd;
  // string server;
  // string port;
  // // string temp1;
  // // string temp2; 
  // string delimiter = ":";

  

  // server = fs_loc.substr(0, fs_loc.find(delimiter));
  // port = fs_loc.substr(fs_loc.find(delimiter)+1);


	string port, server;
	stringstream stream(fs_loc);
	getline(stream, server,':');
	getline(stream, port, '\0');
	cout << "Server Name: " << server << " Port: " << port << endl;

  
  // cout << "Server: " << server << " Port: " << port << endl;


  addrinfo* addr, hints; 
  
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM; 


  if (getaddrinfo(server.c_str(), port.c_str(), &hints, &addr) !=0)
  {
    cerr <<" getaddrinfo failed... " << endl;
		return;

  }

  cs_sock = socket(AF_INET, SOCK_STREAM, 0); 
  
  if (connect(cs_sock, addr->ai_addr, addr->ai_addrlen) == -1){
    cout << "Connection failed..." << endl;
  } else{
    cout << "Connection has been established..." << endl; 
  }
  freeaddrinfo(addr);

  is_mounted = true;  
}

// Unmount the network file system if it was mounted
void Shell::unmountNFS() {
	// close the socket if it was mounted
  if(is_mounted){
    close(cs_sock); 
  } else{
    cout << "No netowrk file system is currently mounted" << endl;
  }
}

// Remote procedure call on mkdir
void Shell::mkdir_rpc(string dname) {

  string message;
  string response;  

  message = ("mkdir " + dname +"\r\n");
  
  sendMessage(message); 

  receive(response); 

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    receive(response);
  } else{
    cout << response;
    receive(response);
    receive(response);
  }
} 

// Remote procedure call on cd
void Shell::cd_rpc(string dname) {

  string message;
  string response;  

  message = ("cd " + dname +"\r\n");

  sendMessage(message);

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    receive(response);
  } else{
    cout << response;
    receive(response);
    receive(response);
  }
}

// Remote procedure call on home
void Shell::home_rpc() {

  string message;
  string response;  

  message = ("home\r\n");

  sendMessage(message);

  receive(response);
  cout << response;
  receive(response);
  receive(response);

}

// Remote procedure call on rmdir
void Shell::rmdir_rpc(string dname) {
  string message;
  string response;  

  message = ("rmdir " + dname +"\r\n");

  sendMessage(message);

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    receive(response);
  } else{
    cout << response;
    receive(response);
    receive(response);
  }

}

// Remote procedure call on ls
void Shell::ls_rpc() {

  string message;
  string response;  

  message = ("ls\r\n");

  sendMessage(message);

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    receive(response);
    receive(response); 
    receive(response); 
    cout << response;
  } else{
    receive(response);
    receive(response); 
    receive(response);
    cout << response;
  }
   
  
}


// Remote procedure call on create
void Shell::create_rpc(string fname) {

  string message;
  string response;  

  message = ("create " + fname +"\r\n");

  sendMessage(message);

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    receive(response);
  } else{
    cout << response;
    receive(response);
    receive(response);
  }
   
}


// Remote procedure call on append
void Shell::append_rpc(string fname, string data) {

  string message;
  

  // message = ("append " + fname + " " + data + "\r\n");

  sendMessage("append " + fname + " " + data + "\r\n");

  string response; 

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    receive(response);
  } else{
    cout << response;
    receive(response);
    receive(response);
  }
}

// Remote procesure call on cat
void Shell::cat_rpc(string fname) {
  string message;
  string response;  

  message = ("cat " + fname +"\r\n");

  sendMessage(message);

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    cout << response;
    receive(response);
    receive(response);
    cout << response;
  } else{
    cout << response;
    receive(response);
    receive(response);
  }
  
}

// Remote procedure call on head
void Shell::head_rpc(string fname, int n) {
  // to implement
  string message;
  string response;  

  message = ("head " + fname + " " + to_string(n) + "\r\n");

  sendMessage(message);

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    cout << response;
    receive(response);
    receive(response);
    cout << response;
  } else{
    cout << response;
    receive(response);
    receive(response);
  }
  
}

// Remote procedure call on rm
void Shell::rm_rpc(string fname) {
  string message;
  string response;  

  message = ("rm " + fname +"\r\n");

  sendMessage(message);

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    receive(response); 
  } else{
    cout << response;
    receive(response);
    receive(response);
  }
   
}

// Remote procedure call on stat
void Shell::stat_rpc(string fname) {
  // to implement
  string message;
  string response;  

  message = ("stat " + fname +"\r\n");

  sendMessage(message);

  receive(response);

  if(strcmp(response.c_str(), "200 OK\r\n") == 0)
  {
    cout << response;
    receive(response);
    receive(response);
    receive(response);
      //receive(response);
    cout << response;
  } else{
    cout << response;
    receive(response);
    receive(response);
      receive(response);
  }
  
}

// Executes the shell until the user quits.
void Shell::run()
{
  // make sure that the file system is mounted
  if (!is_mounted)
 	return; 
  
  // continue until the user quits
  bool user_quit = false;
  while (!user_quit) {

    // print prompt and get command line
    string command_str;
    cout << PROMPT_STRING;
    getline(cin, command_str);

    // execute the command
    user_quit = execute_command(command_str);
  }

  // unmount the file system
  unmountNFS();
}

// Execute a script.
void Shell::run_script(char *file_name)
{
  // make sure that the file system is mounted
  if (!is_mounted)
  	return;
  // open script file
  ifstream infile;
  infile.open(file_name);
  if (infile.fail()) {
    cerr << "Could not open script file" << endl;
    return;
  }


  // execute each line in the script
  bool user_quit = false;
  string command_str;
  getline(infile, command_str, '\n');
  while (!infile.eof() && !user_quit) {
    cout << PROMPT_STRING << command_str << endl;
    user_quit = execute_command(command_str);
    getline(infile, command_str);
  }

  // clean up
  unmountNFS();
  infile.close();
}


// Executes the command. Returns true for quit and false otherwise.
bool Shell::execute_command(string command_str)
{
  // parse the command line
  struct Command command = parse_command(command_str);

  // look for the matching command
  if (command.name == "") {
    return false;
  }
  else if (command.name == "mkdir") {
    mkdir_rpc(command.file_name);
  }
  else if (command.name == "cd") {
    cd_rpc(command.file_name);
  }
  else if (command.name == "home") {
    home_rpc();
  }
  else if (command.name == "rmdir") {
    rmdir_rpc(command.file_name);
  }
  else if (command.name == "ls") {
    ls_rpc();
  }
  else if (command.name == "create") {
    create_rpc(command.file_name);
  }
  else if (command.name == "append") {
    append_rpc(command.file_name, command.append_data);
  }
  else if (command.name == "cat") {
    cat_rpc(command.file_name);
  }
  else if (command.name == "head") {
    errno = 0;
    unsigned long n = strtoul(command.append_data.c_str(), NULL, 0);
    if (0 == errno) {
      head_rpc(command.file_name, n);
    } else {
      cerr << "Invalid command line: " << command.append_data;
      cerr << " is not a valid number of bytes" << endl;
      return false;
    }
  }
  else if (command.name == "rm") {
    rm_rpc(command.file_name);
  }
  else if (command.name == "stat") {
    stat_rpc(command.file_name);
  }
  else if (command.name == "quit") {
    return true;
  }

  return false;
}

// Parses a command line into a command struct. Returned name is blank
// for invalid command lines.
Shell::Command Shell::parse_command(string command_str)
{
  // empty command struct returned for errors
  struct Command empty = {"", "", ""};

  // grab each of the tokens (if they exist)
  struct Command command;
  istringstream ss(command_str);
  int num_tokens = 0;
  if (ss >> command.name) {
    num_tokens++;
    if (ss >> command.file_name) {
      num_tokens++;
      if (ss >> command.append_data) {
        num_tokens++;
        string junk;
        if (ss >> junk) {
          num_tokens++;
        }
      }
    }
  }

  // Check for empty command line
  if (num_tokens == 0) {
    return empty;
  }
    
  // Check for invalid command lines
  if (command.name == "ls" ||
      command.name == "home" ||
      command.name == "quit")
  {
    if (num_tokens != 1) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "mkdir" ||
      command.name == "cd"    ||
      command.name == "rmdir" ||
      command.name == "create"||
      command.name == "cat"   ||
      command.name == "rm"    ||
      command.name == "stat")
  {
    if (num_tokens != 2) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "append" || command.name == "head")
  {
    if (num_tokens != 3) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else {
    cerr << "Invalid command line: " << command.name;
    cerr << " is not a command" << endl; 
    return empty;
  } 

  return command;
}


// Helper functions

 bool Shell::sendMessage(string message)
 {
  int length = message.length(); 
  int count = 0;
	int byteSent = 0;	

  for (int i = 0; i < length; i++)
  {
    char msg = message[i];
    count = 0;
    byteSent = 0;

    while (count < sizeof(char))
    {
      if((byteSent = send(cs_sock, (void*)&msg, sizeof(char), 0)) == -1)
        perror("Error sending message");

      count += byteSent;
    }
  }
}


bool Shell::receive(string &response)
{
  vector<char> msg;
  int byteSent = 0;
  int count = 0;
  char buf;
  bool done = false;
  char endP;

    while (!done)
    {
       count = 0;

        while (count < sizeof(char))
        {
            if ((byteSent = recv(cs_sock, (void*)&buf, sizeof(char), 0)) == -1){
                return false;
            }
            count += byteSent;
        }

        if (buf == '\n' && (endP == '\r'))
            done = true;
        
        endP = buf;
        msg.push_back(buf);
    }
  

    response = string(msg.begin(), msg.end());

    return true;
}
