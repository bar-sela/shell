
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/un.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdbool.h>

#define SA struct sockaddr
#define PORT 8080

///https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program - DIR PART 
//https://www.geeksforgeeks.org/exec-family-of-functions-in-c/ -- explein about exevc 
//https://www.geeksforgeeks.org/wait-system-call-c/ -- about the waitpid part ' -- the else part 
//https://man7.org/linux/man-pages/man2/unlink.2.html -- explein about unlink 




int main(){
    char user_input[1024];
    int sock;   // TCP socket 
    bool tcpConnected = false;

    while (true){
        printf("enter your command:\n");
          fgets(user_input, 1024, stdin);
          user_input[strlen(user_input)-1] = '\0';

          /// סעיף א
        if(strcmp(user_input,"EXIT") == 0 ){
            if(tcpConnected){
                send(sock, "client left", 20 , 0);
               close(sock);
            }
            break;   
        }
        /// סעיף ב' 
        else if(strcmp(user_input,"getcwd") == 0 ){
            char directory[1024];
            getcwd(directory,1024);
            if(!tcpConnected)
              printf(" the directory is:%s\n",directory);
            else 
              send(sock,directory, sizeof(directory),0);   
        }
                            // סעיף ג' 
        // The function "creates a pointer to the cell in the input string 
        //- in the 5th index (after the echo is over) and prints or transmits to the server the result
        else if(strncmp(user_input,"ECHO",4) == 0 ){
               char message[1024] ;
               char*p = &user_input[5];
                int i = 0 ;
                int k = 0 ;
                while (p[i] != '\0'){
                    if(! tcpConnected){    
                       printf("%c",p[i]);
                    }
                    else {
                        message[k] = p[i];   
                    }
                      i++; k++; 
                }
                    printf("\n");
                    message[k] = '\0';
                    if(tcpConnected)
                       send(sock, message,strlen(message),0);
        }
       

                      // סעיף ד 
                // Create a socket and connect to a server
       else if(strcmp(user_input, "TCP PORT") == 0 ){
                    int connfd;
                    struct sockaddr_in servaddr, cli;
                    sock = socket(AF_INET, SOCK_STREAM, 0);
                    if (sock == -1) {
                        printf("socket creation failed...\n");
                        exit(0);
                    }
                    else
                        printf("Socket successfully created..\n");
                    bzero(&servaddr, sizeof(servaddr));
                
                    // assign IP, PORT
                    servaddr.sin_family = AF_INET;
                    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                    servaddr.sin_port = htons(PORT);
                
                    // connect the client socket to server socket
                    if (connect(sock, (SA*)&servaddr, sizeof(servaddr)) != 0) {
                        printf("connection with the server failed...\n");
                        exit(0);
                    }
                    else{
                        tcpConnected = true;
                        send(sock,"hello from client",20,0);
                        printf("you are connected to the server\n");
                    }
        }
           // סעיף ה'
           //Disconnect from the server
        else if(strcmp(user_input,"LOCAL")==0){
            if(tcpConnected == false)
                continue;
            else {
                send(sock, "client left", 20 , 0);
                close(sock);
                tcpConnected = false;
            }    
        }


    /// סעיף ו
    // this function is almost unchanged from stackoverflow at the begining
    // View on the files which are in our folder

        else if(strcmp(user_input, "DIR") == 0){
            DIR *d;
            struct dirent *dir;
            d = opendir(".");
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                 if (!tcpConnected)
                   printf("%s\n", dir->d_name);
                else{
                   send(sock,dir->d_name,sizeof(dir->d_name) ,0);
                   sleep(1);
                }
                }
                closedir(d);
            }
        }
  
          // סעיף ז 
        //Skip the CD part of the input and enter the folder described in the rest of the string
        //  chdir is a system call 
        else if(strncmp(user_input,"CD",2)== 0 ){
            char* p = &user_input[3];
              if (chdir(p) != 0) 
                  if( !tcpConnected)
                    perror("chdir() to /error failed\n");
                  else 
                    send(sock,"chdir() to /error failed\n",40,0);
             else
                if (!tcpConnected)
                    printf("entered to a new directory\n");   
                else 
                   send(sock,"entered to a new directory\n",40,0);     
        }


          // סעיף י'  

        else if(strncmp(user_input,"COPY",4)==0){
          //  Copy the names of the files given into as part of the input into variables
          //   we are only using here library functions - not system calls 
                 char src[1024] ;
                 char dest[1024];
                 char* token = strtok(user_input, " ");
                 int k = 0 ;
                while (token != NULL) {
                    if(k == 1 )
                       strcpy(src,token);
                    if(k==2 )
                        strcpy(dest,token);   
                    token = strtok(NULL, " ");
                    k ++;
        }
            char c ;
            FILE * src_f= fopen(src, "rb");
            FILE * dest_f = fopen(dest, "ab+");
                 /// Find out if the files provided do exist
            if(src_f == NULL || dest_f == NULL){
                if(!tcpConnected)
                     printf("at least one file dosent exist\n");
                else 
                    send(sock,"at least one file dosent exist",40,0);
                continue;          
            }
            //Copy the content
            while ((fread(&c,1,1,src_f))!= 0){
                fwrite(&c,1,1,dest_f);
            }
             fclose(dest_f);
             fclose(src_f);  
    }

    // סעיף יא 
    // here we are deleting an exist file from our directory . in case the file wthich gien dosent exist - 
    // we will continue to the nex itiration of the head while loop.
  
    //in out implementaion we are also using unlink which is a system call   
    else if(strncmp(user_input, "DELETE",6) == 0 ){
        //Check whether the file submitted for deletion actually exists
             char* token = strtok(user_input, " ");
             token = strtok(NULL, " ");
             if (token == NULL){
                 if (!tcpConnected)
                     printf("you didnt pass a directory\n");
                 else 
                    send(sock,"you didnt pass a directory",40,0);    
                 continue;    
             }
             // if does exist - delete it 
             else {
                 int num = unlink(token);
                 if(num ==-1 ){  /// an error has happend 
                   if (!tcpConnected)
                        printf("didnt delete the directory\n");
                    else
                      send(sock,"didnt delete the directory",40,0);
                 }
                 else {  // success
                    if (!tcpConnected)
                       printf("deleting have been succsessed\n");   
                    else
                        send(sock,"deleting have been succsessed",40,0);
                 }
             }
    }
    /*
    In the case of any other input string given - we were given instructions for two operations
    . One very short action of executing the command given by the system function function and a second action
     - through a son process and waiting for the parent process that the son process will complete the task given to it.
     */
    else{
        // סעיף ח
         //system(user_input);  system is a system call 

         /// סעיף ט
         int cid = fork();
         int status;
          if(cid==0 ){ //only child commiting
             int k = 0 ;
             char* argv[250]; 
            char* token = strtok(user_input, " ");
            while (token != NULL) {
                    argv[k] = token;
                    token = strtok(NULL, " ");
                    k++;
            }
            argv[k] =NULL;
            execvp(argv[0],argv);
          }
            if(cid > 0 ){
                waitpid(cid,&status,0); // wait for the child proccess to finish
                if (WIFEXITED(status)){ // If the son process successfully performed the task without single interrupted
                   if (!tcpConnected)
                       printf("Exit status: %d\n", WEXITSTATUS(status));
                  else
                       send(sock,"chiled proccess has done his task!",40,0);
                }
                else if (WIFSIGNALED(status)) // The son process did not successfully perform the task - signal interrupted
                   if (!tcpConnected)
                      psignal(WTERMSIG(status), "Exit signal");
                    else
                         send(sock,"an error has happend during chiled execute!",40,0);
                   }
            }
        }
    }

