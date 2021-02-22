#include <stdio.h>
#include <string.h>    // for strlen
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>   // for threading, link with lpthread
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>    // for write

void *message_sender();
void *message_receiver();

int client_sock_desc = 0;
char name[140];
char phonenumber[15];
bool isOnline = true;

int main(int argc, char **argv)
{
    printf("Enter your name: ");
    fgets(name, 140, stdin);
    printf("Enter your phone number like '5xxxxxxxxx': ");
    fgets(phonenumber, 15, stdin);
    puts(name);
    puts(phonenumber);

    struct sockaddr_in client;

    client_sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }
     
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons(3205);

    if (connect(client_sock_desc, (struct sockaddr *)&client, sizeof(client))<0)
    {
        puts("Connection error");
        return 1;
    }

    strcat(name,phonenumber);
    send(client_sock_desc, name, sizeof(name), 0);
    

    pthread_t send,receive;
    pthread_create(&send,NULL,message_sender,NULL);
    pthread_create(&receive,NULL,message_receiver,NULL);
    pthread_join(send,NULL);
    pthread_join(receive,NULL);
    
    close(client_sock_desc);
    
}
void *message_sender(){
    while(true){
        char input[140];
        fgets(input,140, stdin);
        if(strcmp(input,"-exit\n")==0){            
            isOnline=false;       
            //Send server to exit command
            if(send(client_sock_desc,input,strlen(input)+1,0)<0){
                puts("Send failedn\n");
            }                        
            break;
        }
        else{
            if(write(client_sock_desc,input,strlen(input)+1)<0){
                puts("Sending failedn\n");
            }
        } 
    }
}
void *message_receiver(){
    while(true){
        char received_msg[140];
        if(!isOnline){                    
            break;
        }
        if(recv(client_sock_desc,received_msg,140,0)<0){
            puts("Receiving failed");
        }
        else{
            puts(received_msg);
        }
    }
}