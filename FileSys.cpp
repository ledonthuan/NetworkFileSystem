// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <string.h>

// mounts the file system
void FileSys::mount(int sock)
{
  bfs.mount();
  curr_dir = 1; //by default curr directory is home directory, in disk block #1
  fs_sock = sock; //use this socket to receive file system operations from the client and send back response messages
}

// unmounts the file system
void FileSys::unmount()
{
  bfs.unmount();
  close(fs_sock);
}

// make a directory
void FileSys::mkdir(const char *name)
{
    dirblock_t curr;
    bfs.read_block(curr_dir, (void*) &curr);
    
    // check file name size
    if (strlen(name) > MAX_FNAME_SIZE){
        sendMessage("504 File name is too long\r\n");
        sendMessage("Length:0\r\n");
        sendMessage("\r\n");
        return;
    }
    
    // check if file name exists
    for (unsigned int i = 0; i < curr.num_entries; i++){
        if(strcmp(curr.dir_entries[i].name, name) == 0){
            sendMessage("502 File exists\r\n");
            sendMessage("Length:0\r\n");
            sendMessage("\r\n");
            return;
        }
    }
    
    if (curr.num_entries == MAX_DIR_ENTRIES){
        sendMessage("506 Directory is full\r\n");
        sendMessage("Length:0\r\n");
        sendMessage("\r\n");
        return;
    }
    
    //aquire free block
    short newBlock = bfs.get_free_block();
    //free block error handling
    if(newBlock == 0){
        sendMessage("505 Disk is full\r\n");
        sendMessage("Length:0\r\n");
        sendMessage("\r\n");
        return;
    }
    
    curr.dir_entries[curr.num_entries].block_num = newBlock;
    strcpy(curr.dir_entries[curr.num_entries].name, name);
    curr.num_entries++;
    
    struct dirblock_t newDirectory;
    newDirectory.magic = DIR_MAGIC_NUM;
    newDirectory.num_entries = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++){
        newDirectory.dir_entries[i].block_num = 0;
    }
    
    // write blocks back to disk
    bfs.write_block(curr_dir, (void *) &curr);
    bfs.write_block(newBlock, (void *) &newDirectory);
    
    sendMessage("200 OK\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    return;
    
}

// switch to a directory
void FileSys::cd(const char *name)
{
    dirblock_t curr;
    dirblock_t insideDir;
    
    bfs.read_block(curr_dir, (void *) &curr);
    
    for (int i = 0; i < curr.num_entries; i++){
        if (strcmp(curr.dir_entries[i].name, name) == 0){
            bfs.read_block(curr.dir_entries[i].block_num, (void *) &insideDir);
            if (insideDir.magic == DIR_MAGIC_NUM){
                curr_dir = curr.dir_entries[i].block_num;
                sendMessage("200 OK\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            } else {
                sendMessage("500 File is not a directory\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            }
        }
    }
    sendMessage("503 File does not exist\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    return;
}

// switch to home directory
void FileSys::home()
{
    curr_dir = 1;
    
    sendMessage("200 OK\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
}

// remove a directory
void FileSys::rmdir(const char *name)
{
    dirblock_t curr;
    dirblock_t insideDir;
    
    bfs.read_block(curr_dir, (void *) &curr);
    
    for (int i = 0; i < curr.num_entries; i++){
        if (strcmp(curr.dir_entries[i].name, name) == 0){
            bfs.read_block(curr.dir_entries[i].block_num, (void *) &insideDir);
            if (insideDir.magic == DIR_MAGIC_NUM){
                if (insideDir.num_entries != 0){
                    sendMessage("507 Directory is not empty\r\n");
                    sendMessage("Length:0\r\n");
                    sendMessage("\r\n");
                    return;
                }
                
                bfs.reclaim_block(curr.dir_entries[i].block_num);
                
                for(int j = i; j < curr.num_entries; j++){
                    curr.dir_entries[j] = curr.dir_entries[j+1];
                }
                curr.num_entries--;
                
                bfs.write_block(curr_dir, (void *) &curr);
                
                sendMessage("200 OK\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
                
            } else {
                sendMessage("500 File is not a directory\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            }
        }
    }
    sendMessage("503 File does not exist\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    return;
}

// list the contents of curr directory
void FileSys::ls()
{
    int length;
    string message, list, messageLen;
    
    // struct to read curr directory properities into
    dirblock_t curr, insideDir;

    // read curr directory block
    bfs.read_block(curr_dir, (void *) &curr);
    
    if (curr.num_entries == 0) {
        
        // set up message
        message = "Empty Directory";
        length = message.length();
        messageLen = "Length:" + to_string(length) + "\r\n";
        
        // send response
        sendMessage("200 OK\r\n");
        sendMessage(messageLen);
        sendMessage("\r\n");
        sendMessage("Empty Directory\r\n");
        return;
    }
    
    message = "";
    // read names of files in directory
    for (int i = 0; i < curr.num_entries; i++) {

        bfs.read_block(curr.dir_entries[i].block_num, (void *) &insideDir);
        
        if (insideDir.magic == DIR_MAGIC_NUM) { // is directory
            message = message + strcat(curr.dir_entries[i].name, "/") + " ";
        }
        else { //is inode
            message = message + curr.dir_entries[i].name + " ";
        }
    }
    
    length = message.length();
    messageLen = "Length:" + to_string(length) + "\r\n";
    message = message + "\r\n";
    
    sendMessage("200 OK\r\n");
    sendMessage(messageLen);
    sendMessage("\r\n");
    sendMessage(message);
    
    return;
}

// create an empty data file
void FileSys::create(const char *name)
{
    dirblock_t curr;
    bfs.read_block(curr_dir, (void*) &curr);
    
    // check file name size
    if (strlen(name) > MAX_FNAME_SIZE){
        sendMessage("504 File name is too long\r\n");
        sendMessage("Length:0\r\n");
        sendMessage("\r\n");
        return;
    }
    
    // check if file name exists
    for (unsigned int i = 0; i < curr.num_entries; i++){
        if(strcmp(curr.dir_entries[i].name, name) == 0){
            sendMessage("502 File exists\r\n");
            sendMessage("Length:0\r\n");
            sendMessage("\r\n");
            return;
        }
    }
    
    if (curr.num_entries == MAX_DIR_ENTRIES){
        sendMessage("506 Directory is full\r\n");
        sendMessage("Length:0\r\n");
        sendMessage("\r\n");
        return;
    }
    
    //aquire free block
    short newBlock = bfs.get_free_block();
    //free block error handling
    if(newBlock == 0){
        sendMessage("505 Disk is full\r\n");
        sendMessage("Length:0\r\n");
        sendMessage("\r\n");
        return;
    }
    
    curr.dir_entries[curr.num_entries].block_num = newBlock;
    strcpy(curr.dir_entries[curr.num_entries].name, name);
    curr.num_entries++;
    
    struct inode_t newInode;
    newInode.magic = INODE_MAGIC_NUM;
    newInode.size = 0;
    for (int i = 0; i < MAX_DATA_BLOCKS; i++){
        newInode.blocks[i] = 0;
    }
    
    bfs.write_block(curr_dir, (void *) &curr);
    bfs.write_block(newBlock, (void *) &newInode);
    
    sendMessage("200 OK\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    
    return;
}

// append data to a data file
void FileSys::append(const char *name, const char *data)
{
    int dataSize = strlen(data);
    if (dataSize > MAX_FILE_SIZE){
        sendMessage("508 Append exceeds maximum file size\r\n");
        sendMessage("Length:0\r\n");
        sendMessage("\r\n");
        return;
        
    }
    
    dirblock_t curr;
    inode_t inside;

    string name1 = string(name);
	name1 = string(" ") + name1;
    
    bfs.read_block(curr_dir, (void*) &curr);
    // find name of file to append to
    for (int i = 0; i < curr.num_entries; i++){
        if (strcmp(curr.dir_entries[i].name, name1.c_str()) == 0){
            bfs.read_block(curr.dir_entries[i].block_num, (void*) &inside);
            
            if (inside.magic == DIR_MAGIC_NUM){
                sendMessage("501 File is a directory\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            } else if (inside.magic == INODE_MAGIC_NUM){
                // now check curr size
                int newBlock, addBlocks, count, dataIndex, index;

                count = -1;
                index = 0;
                
                // cases: size is 0; size is inbetween
                if (inside.size == 0) {

                    // determine number of blocks needed
                    addBlocks = 1 + (dataSize / BLOCK_SIZE);
                    
                    // initialize the datablock_t array
                    datablock_t* array = new datablock_t[addBlocks];
                    
                    // initialize first block needed
                    newBlock = bfs.get_free_block();
                    
                    if (newBlock == 0) {
                        sendMessage("505 Disk is full\r\n");
                        sendMessage("Length:0\r\n");
                        sendMessage("\r\n");
                        return;
                    }
                    
                    dataIndex = 0;
                    inside.blocks[dataIndex] = newBlock;

                    for (int character = 0; character < dataSize; character++) {

                        // if its time for a new block
                        if (character % BLOCK_SIZE == 0) {

                            if (count != -1) {
                                // write this last block back to disk
                                bfs.write_block(newBlock, (void *) &array[index]);
                                bfs.write_block(curr_dir, (void *) &curr);
                                bfs.write_block(curr.dir_entries[i].block_num, (void *) &inside);
                            
                                // update block array idnex
                                index++;
                                
                                // get next free block and entry
                                newBlock = bfs.get_free_block();
                                
                                if (newBlock == 0) {
                                    sendMessage("505 Disk is full\r\n");
                                    sendMessage("Length:0\r\n");
                                    sendMessage("\r\n");
                                    return;
                                }
                                
                                dataIndex++;
                                inside.blocks[dataIndex] = newBlock;
                                
                                // reset count for new block
                                count = 0;
                            }
                            else {
                                count = 0;
                            }
                        }
                        
                        // write and update count
                        array[index].data[count] = data[character];
                        count++;
                    }
                    
                    // write this last block back to disk
                    inside.size = dataSize;
                    bfs.write_block(newBlock, (void *) &array[addBlocks - 1]);
                    bfs.write_block(curr_dir, (void *) &curr);
                    bfs.write_block(curr.dir_entries[i].block_num, (void *) &inside);
                }
                else {

                    // init
                    int remaining, last_block, last_position, count;
                    datablock_t quick;

                    // check if file + new string is too much
                    if (inside.size + dataSize > MAX_FILE_SIZE) {
                        sendMessage("508 Append exceeds maximum file size\r\n");
                        sendMessage("Length:0\r\n");
                        sendMessage("\r\n");
                        return;
                    }
                    
                    // find the curr block of the tail of the file
                    dataIndex = inside.size / BLOCK_SIZE;
                    last_block = inside.blocks[dataIndex];
                    last_position = inside.size % BLOCK_SIZE;
                    
                    // if we can write then check how many new blocks we will need.
                    if (last_position + dataSize <  BLOCK_SIZE) {
                        
                        // new string will fit on currly allocated block; quickly write
                        addBlocks = 0;
                        bfs.read_block(last_block, (void *) &quick);
                        
                        for (int character = last_position; character < (last_position + dataSize); character++) {
                            quick.data[character] = data[character - last_position];
                        }
                        
                        // write everything back to disk
                        inside.size = inside.size + dataSize;
                        bfs.write_block(last_block, (void *) &quick);
                        bfs.write_block(curr_dir, (void *) &curr);
                        bfs.write_block(curr.dir_entries[i].block_num, (void *) &inside);
                    }
                    else {

                        // compute remaining space and blocks needed
                        remaining = dataSize - (inside.size % BLOCK_SIZE);
                        addBlocks = 1 + (remaining / BLOCK_SIZE);
                        
                        // initialize the datablock_t array
                        datablock_t* array = new datablock_t[addBlocks];
                        
                        // read the curr block into array[0]
                        bfs.read_block(last_block, (void *) &array[0]);
                        
                        if (newBlock == 0) {
                            sendMessage("505 Disk is full\r\n");
                            sendMessage("Length:0\r\n");
                            sendMessage("\r\n");
                            return;
                        }
                        
                        count = last_position;
                        index = 0;
                        
                        for (int character = 0; character < dataSize; character++) {

                            // if its time for a new block
                            if (count % BLOCK_SIZE == 0) {

                                // write this last block back to disk
                                bfs.write_block(last_block, (void *) &array[index]);
                                bfs.write_block(curr_dir, (void *) &curr);
                                bfs.write_block(curr.dir_entries[i].block_num, (void *) &inside);
                            
                                // update block array idnex
                                index++;

                                // get next free block and entry
                                last_block = bfs.get_free_block();
                                
                                if (newBlock == 0) {
                                    sendMessage("505 Disk is full\r\n");
                                    sendMessage("Length:0\r\n");
                                    sendMessage("\r\n");
                                    return;
                                }
                                
                                dataIndex++;
                                inside.blocks[dataIndex] = last_block;

                                // reset count for new block
                                count = 0;
                
                            }
                            
                            // write and update count
                            array[index].data[count] = data[character];
                            count++;
                        }
                        
                        // write this last block back to disk
                        inside.size = inside.size + dataSize;
                        bfs.write_block(last_block, (void *) &array[index]);
                        bfs.write_block(curr_dir, (void *) &curr);
                        bfs.write_block(curr.dir_entries[i].block_num, (void *) &inside);
                    }
                }
                
                sendMessage("200 OK\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            }
        }
    }
    
    sendMessage("503 File does not exist\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    return;
}

// display the contents of a data file
void FileSys::cat(const char *name)
{
    dirblock_t curr;
    inode_t inside;
    datablock_t newBlock;
    
    bfs.read_block(curr_dir, (void *) &curr);
    
    for (int i = 0; i < curr.num_entries; i++){
        if (strcmp(curr.dir_entries[i].name, name) == 0){
            bfs.read_block(curr.dir_entries[i].block_num, (void *) &inside);
            if (inside.magic == DIR_MAGIC_NUM){
                sendMessage("501 File is a directory\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            } else {
                int indexBlocks = 0;
                int characterIndex = 0;
                string message, len;
                message = "";
                int length;
                for (int j = 0; j < inside.size; j++) {
                    if (j % BLOCK_SIZE == 0) {
                        bfs.read_block(inside.blocks[indexBlocks], (void *) &newBlock);
                        indexBlocks++;
                        characterIndex = 0;
                    }
                    message = message + newBlock.data[characterIndex];
                    characterIndex++;
                }
                
                //setup
                length = message.length();
                len = "Length:" + to_string(length) + "\r\n";
                message = message + "\r\n";
                // send
                sendMessage("200 OK\r\n");
                sendMessage(len);
                sendMessage("\r\n");
                sendMessage(message);
                return;
            }
        }
    }
    sendMessage("503 File does not exist\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    return;
}

// display the first N bytes of the file
void FileSys::head(const char *name, unsigned int n)
{
    dirblock_t curr;
    inode_t inside;
    
    string name1 = string(name);
	name1 = string(" ") + name1;

    bfs.read_block(curr_dir, (void *) &curr);
    datablock_t newBlock;
    for (int i = 0; i < curr.num_entries; i++){
        if (strcmp(curr.dir_entries[i].name, name1.c_str()) == 0){
            bfs.read_block(curr.dir_entries[i].block_num, (void *) &inside);
            if (inside.magic == DIR_MAGIC_NUM){
                sendMessage("501 File is a directory\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            } else {
                int indexBlocks = 0;
                int characterIndex = 0;
                
                string message;
                string len;
                int length;
                message = "";
                for (int j = 0; j < n; j++){
                    if (j % BLOCK_SIZE == 0){
                        bfs.read_block(inside.blocks[indexBlocks], (void *) &newBlock);
                        indexBlocks++;
                        characterIndex = 0;
                    }
                    message = message + newBlock.data[characterIndex];
                    characterIndex++;
                }
                length = message.length();
                len = "Length:" + to_string(length) + "\r\n";
                message = message + "\r\n";
                
                sendMessage("200 OK\r\n");
                sendMessage(len);
                sendMessage("\r\n");
                sendMessage(message);
                return;
            }
        }
    }
    sendMessage("503 File does not exist\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    return;
}

// delete a data file
void FileSys::rm(const char *name)
{
    int numBlocks;
    dirblock_t curr;
    inode_t inside;
    
    bfs.read_block(curr_dir, (void *) &curr);
    
    for (int i = 0; i < curr.num_entries; i++){
        if (strcmp(curr.dir_entries[i].name, name) == 0){
            bfs.read_block(curr.dir_entries[i].block_num, (void *) &inside);
            if (inside.magic == DIR_MAGIC_NUM){
                sendMessage("501 File is a directory\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            } else {
                if (inside.size > 0) {
                    // one for first block, variable for rest
                    numBlocks = 1 + inside.size / BLOCK_SIZE;
                }
                else {
                    // know the file is empty, so only inode blok
                    numBlocks = 1;
                }

                for (short j = 0; j < numBlocks; j++) {
                    bfs.reclaim_block(inside.blocks[i]);
                }
                
                for(int k = i; k < curr.num_entries; k++){
                    curr.dir_entries[k] = curr.dir_entries[k+1];
                }
                bfs.reclaim_block(curr.dir_entries[i].block_num);
                curr.num_entries--;
                bfs.write_block(curr_dir, (void *) &curr);
                
                sendMessage("200 OK\r\n");
                sendMessage("Length:0\r\n");
                sendMessage("\r\n");
                return;
            }
        }
    }
    sendMessage("503 File does not exist\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    return;
}

// display stats about file or directory
void FileSys::stat(const char *name)
{
    dirblock_t curr;
    dirblock_t insideDir;
    inode_t inside;
    string message;
    int length;
    string len;
    bfs.read_block(curr_dir, (void *) &curr);
    bool isDir;
    for (int i = 0; i < curr.num_entries; i++){
        if (strcmp(curr.dir_entries[i].name, name) == 0){
            isDir = isADirectory(&curr, i);
            if (isDir){
                bfs.read_block(curr.dir_entries[i].block_num, (void *) &insideDir);
                message = string("Directory name: ") + string(curr.dir_entries[i].name) + string("\n");
                message = message + string("Directory block: ") + to_string(curr.dir_entries[i].block_num) + "\r\n";
                        
                // print results
                length = message.length();
                len = "Length:" + to_string(length) + "\r\n";
                
                
                sendMessage("200 OK\r\n");
                sendMessage(len);
                sendMessage("\r\n");
                sendMessage(message);

                return;
            } else {
                bfs.read_block(curr.dir_entries[i].block_num, (void *) &inside);
                int numBlocks;
                // determine number of blocks
                if (inside.size > 0) {
                    // one for inode, one for first block, variable for rest
                    numBlocks = 2 + inside.size / BLOCK_SIZE;
                }
                else {
                    // know the file is empty, so only inode blok
                    numBlocks = 1;
                }
                
                // print results
                message = string("Inode block: ") + to_string(curr.dir_entries[i].block_num) + string("\n");
                message = message + string("Bytes in the file: ") + to_string(inside.size) + string("\n");
                message = message + string("Number of blocks: ") +  to_string(numBlocks) + string("\n");
                message = message + string("First block: ") + to_string(inside.blocks[0]) + string("\r\n");
                
                length = message.length();
                len = "Length:" + to_string(length) + "\r\n";

                sendMessage("200 OK\r\n");
                sendMessage(len);
                sendMessage("\r\n");
                sendMessage(message);
                return;
            }
        }
    }
    sendMessage("503 File does not exist\r\n");
    sendMessage("Length:0\r\n");
    sendMessage("\r\n");
    return;
}











// HELPER FUNCTIONS (optional)
bool FileSys::isADirectory(dirblock_t* curr,  int i){
    dirblock_t check;
    bfs.read_block(curr->dir_entries[i].block_num, (void*) &check);
    if (check.magic == DIR_MAGIC_NUM)
        return true;
    else
        return false;
}

bool FileSys::sendMessage(string message)
{
    for (int i = 0; i < message.length(); i++)
    {
        char messageChar = message[i];
        int bytesSent = 0;
        int checkSent;
        while (bytesSent < sizeof(char))
        {
            checkSent = send(fs_sock, (void*) &messageChar, sizeof(char), 0);
            if (checkSent == -1)
            {
                perror("Error sending message");
            }
            bytesSent += checkSent;
        }
    }
}
