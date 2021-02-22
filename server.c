#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>    // for write
#include <pthread.h>   // for threading, link with lpthread
#include <stdbool.h>

//functions right usage examples............
//-gcreate 5353334060+group1 (other usage causes error)
//-join group1
//-send message1
//-exit group1
//-exit
//-whoami
//JSON FORMAT MESSAGE IS NOT COMPLETED :(
int client_counter = 0; //counting the clients in the server at the same time.
int group_counter = 0;  //counting the groups in the server at the same time.

typedef struct clientstruct
{ //client struct that keeps informaiton af clients to use them in code.
    int clientid;
    char username[140];
    char phonenumber[15];
    int client_socket_desc; //keeps all clients socket number to send messages to all of possible client.
    bool inGroup;           //controls whether is client in any group or not in any group.
    bool isOnline;          //to control exit status.
} clientstruct;

typedef struct groupstruct
{ //group struct that keeps informaiton of groups to use them in code.
    int groupid;
    int clientsingroup_counter; //counting the client in the group.
    char groupName[140];
    char password[140];
    char creator[15];
    clientstruct clientsingroup[15]; //this caused that max client in a group must be 10, to provide group message,
    //members phonenumber must be kept and this should be implemented. Also phonumbers must be unique.
} groupstruct;

struct clientstruct clients[100]; //this caused that max client in server is 100.
struct groupstruct groups[20];    //this caused that max group in server is 20.
//max numbers may be changed from here.
//Also this should be known any created group or client is not deleted from server. They just becomes offline to not use but their info kept until server closed.
void *connection_handler(void *socket_desc)
{
    char client_input[140];  //received input
    char client_output[140]; //sended output
    char client_name[140];
    char client_phone[15];
    int id = client_counter;
    //Get the socket descriptor
    int sock = *((int *)socket_desc);
    //creation of the client with his/her phonenumber and username.
    if (recv(sock, client_input, sizeof(client_input), 0) > 0)
    {
        char *name = strtok(client_input, "\n");
        char *phonenum = strtok(NULL, client_input);
        strcpy(client_name, name);
        strcpy(client_phone, phonenum);
        strcpy(clients[id].username, client_name);
        strcpy(clients[id].phonenumber, client_phone);
        clients[id].client_socket_desc = sock;
        clients[id].clientid = id;
        clients[id].inGroup = false; //if false client is not in any chat group.
        clients[id].isOnline = true; //if true client is online.
        puts(clients[id].username);
        puts(clients[id].phonenumber);
        client_counter++; //increasing client counter to add new clients to the next part of the array.
        strcpy(client_output, client_name);
        strcat(client_output, " entered server with phone number: ");
        strcat(client_output, client_phone);
        write(sock, client_output, strlen(client_output) + 1);
    }
    else
    {
        strcpy(client_output, "Server joinning error.");
        write(sock, client_output, strlen(client_output) + 1);
    }

    while (recv(sock, client_input, sizeof(client_input), 0) > 0 && clients[id].isOnline == true)
    {
        if (strcmp(client_input, "-exit\n") == 0)
        {                                                //exit server section.......................................................................
            clients[id].isOnline = false;                //exitted clients online information becomes false to not use theme again.
            strcpy(client_output, clients[id].username); //string processes to show user left the server.
            strcat(client_output, " left the server");
            write(sock, client_output, strlen(client_output) + 1);
        }
        else if (strcmp(client_input, "-whoami\n") == 0) //show user info with -whoami command section..........................................
        {
            //string process with adding client username and phonenumber to show user again.
            strcpy(client_output, "your username: ");
            strcat(client_output, clients[id].username);
            strcat(client_output, "\nyour phonenumber: ");
            strcat(client_output, clients[id].phonenumber);
            write(sock, client_output, strlen(client_output) + 1);
        }
        else
        {
            char *command = strtok(client_input, " ");
            char *message;
            if (message = strtok(NULL, "\n")) //taking the input except command like -join, -gcreate,.......................................
            {
                puts(command);
                puts(message);
                if (strcmp(command, "-send") == 0)
                {
                    if (clients[id].inGroup == true)
                    {
                        int sendgroup = -1;
                        char *controlphone = clients[id].phonenumber;
                        puts(controlphone);
                        int controlval = id;
                        for (int i = 0; i <= group_counter; i++)
                        {
                            puts(groups[i].groupName);
                            for (int j = 0; j <= groups[i].clientsingroup_counter; j++)
                            {
                                char *temp = groups[i].clientsingroup[j].phonenumber;
                                if (controlval == groups[i].clientsingroup[j].clientid)
                                {
                                    sendgroup = i; //taking the group place to send message to this group.
                                    break;         //ending for loops.
                                }
                            }
                            if (sendgroup == i)
                            {
                                break;
                            }
                        }
                        for (int i = 0; i <= groups[sendgroup].clientsingroup_counter; i++)
                        {
                            if (groups[sendgroup].clientsingroup[i].inGroup == true)
                            {
                                strcpy(client_output, clients[id].username);
                                strcat(client_output, ": ");
                                strcat(client_output, message);
                                write(groups[sendgroup].clientsingroup[i].client_socket_desc, client_output, strlen(client_output) + 1);
                            }
                        }
                    }
                    else
                    {
                        strcpy(client_output, "Server Message: you cant use this command if you not in any group");
                        write(sock, client_output, strlen(client_output) + 1);
                    }
                }
                else if (strcmp(command, "-gcreate") == 0) //create group section.................................................................
                {
                    if (clients[id].inGroup == false)
                    {
                        if (group_counter <= 20) //controls the number of groups.
                        {
                            char *groupcreatorphone = strtok(message, "+");
                            char *groupname = strtok(NULL, "\n");
                            int groupID = group_counter;                  //assignin group id to not confusing with groups array.
                            strcpy(groups[groupID].groupName, groupname); //takes group name from clients' input.
                            //taking group password from user-----------------------------------------------------------------
                            strcpy(client_output, "Please enter a password for group: ");
                            write(sock, client_output, strlen(client_output) + 1);
                            if (recv(sock, client_input, sizeof(client_input), 0) < 0)
                            {
                                strcpy(client_output, "Password assignment error.");
                                write(sock, client_output, strlen(client_output) + 1);
                            }
                            strcpy(groups[groupID].password, client_input);
                            //password assignment to the group is end.---------------------------------------------------------
                            groups[groupID].groupid = groupID;
                            groups[groupID].clientsingroup_counter++;                                             //increasing the clients number in this group.
                            clients[id].inGroup = true;                                                           //makes true to define client is in this group.
                            groups[groupID].clientsingroup[groups[groupID].clientsingroup_counter] = clients[id]; //assigning the creator of this group to the clients of group
                            strcpy(client_output, groups[groupID].groupName);                                     //string processes to alert clienr about group.
                            strcat(client_output, " group were created, you have joined the group");
                            strcpy(groups[groupID].creator, groupcreatorphone);
                            printf("%d\n", groups[groupID].groupid);
                            printf("%d\n", groups[groupID].clientsingroup_counter);
                            puts(groups[groupID].clientsingroup[groups[groupID].clientsingroup_counter].username);
                            puts(groups[groupID].clientsingroup[groups[groupID].clientsingroup_counter].phonenumber);
                            puts(groups[groupID].password);
                            puts(groups[groupID].groupName);
                            puts(groups[groupID].creator);
                            write(sock, client_output, strlen(client_output) + 1);
                            group_counter++;
                        }
                        else
                        {
                            strcpy(client_output, "Server Message: reached the maximum number of groups");
                            write(sock, client_output, strlen(client_output) + 1);
                        }
                    }
                    else
                    {
                        strcpy(client_output, "Server Message: you cant use this command in any group");
                        write(sock, client_output, strlen(client_output) + 1);
                    }
                }
                else if (strcmp(command, "-join") == 0) //join group or user section.............................................................
                {
                    if (clients[id].inGroup == false)
                    {
                        int joinedgroup = -1;
                        for (int i = 0; i <= group_counter; i++)
                        {
                            if (strcmp(message, groups[i].groupName) == 0)
                            {
                                joinedgroup = i; //taking the group place to delete client.
                                puts(groups[i].groupName);
                                break; //ending for loop.
                            }
                        }
                        if (joinedgroup == -1)
                        {
                            //USERNAME JOIN FUNCTION IS NOT COMPLETED :(
                            /*char *usernamegroup; 
                        for (int i = 0; i < client_counter; i++)
                        {
                            if (strcmp(message, clients[i].username)==0)
                            {
                                //strcpy(usernamegroup, clients[i].username);
                            }
                        }
                        puts(usernamegroup);*/

                            strcpy(client_output, "Server Message: unknown groupname");
                            write(sock, client_output, strlen(client_output) + 1);
                        }
                        else
                        {
                            strcpy(client_output, "Please enter group password: ");
                            write(sock, client_output, strlen(client_output) + 1);
                            if (recv(sock, client_input, sizeof(client_input), 0) < 0)
                            {
                                strcpy(client_output, "Password taking error.");
                                write(sock, client_output, strlen(client_output) + 1);
                            }
                            if (strcmp(groups[joinedgroup].password, client_input) == 0)
                            {
                                groups[joinedgroup].clientsingroup_counter++;
                                clients[id].inGroup = true;
                                groups[joinedgroup].clientsingroup[groups[joinedgroup].clientsingroup_counter] = clients[id];
                                strcpy(client_output, clients[id].username);
                                strcat(client_output, " have joined to the ");
                                strcat(client_output, groups[joinedgroup].groupName);

                                for (int i = 0; i <= groups[joinedgroup].clientsingroup_counter; i++)
                                {
                                    if (groups[joinedgroup].clientsingroup[i].inGroup == true)
                                    {
                                        write(groups[joinedgroup].clientsingroup[i].client_socket_desc, client_output, strlen(client_output) + 1);
                                    }
                                }
                            }
                            else
                            {
                                strcpy(client_output, "Wrong Password!");
                                write(sock, client_output, strlen(client_output) + 1);
                            }
                        }
                    }
                    else
                    {
                        strcpy(client_output, "Server Message: you cant use this command in any group");
                        write(sock, client_output, strlen(client_output) + 1);
                    }
                }
                else if (strcmp(command, "-exit") == 0) //exit group section...................................
                {
                    if (clients[id].inGroup == true)
                    {
                        int exittedgroup = -1;
                        for (int i = 0; i <= group_counter; i++)
                        {
                            if (strcmp(message, groups[i].groupName) == 0)
                            {
                                exittedgroup = i; //taking the group place to delete client.
                                break;            //ending for loop.
                            }
                        }
                        if (exittedgroup == -1)
                        { //if user gives a not valid group name...
                            strcpy(client_output, "Server Message: unknown groupname"); 
                            write(sock, client_output, strlen(client_output) + 1);
                        }
                        //exit process starts here......
                        clients[id].inGroup = false; //defines client is not in any group.
                        char *exittedclientphone = clients[id].phonenumber;
                        for (int i = 0; i <= groups[exittedgroup].clientsingroup_counter; i++)
                        {
                            if (strcmp(groups[exittedgroup].clientsingroup[i].phonenumber, exittedclientphone) == 0)
                            {
                                strcpy(client_output, groups[exittedgroup].clientsingroup[i].username);
                                strcat(client_output, " left the group");
                                for (int i = 0; i <= groups[exittedgroup].clientsingroup_counter; i++)
                                {//writes left status of the user to every member of group
                                    if (groups[exittedgroup].clientsingroup[i].inGroup == true)
                                    {
                                        write(groups[exittedgroup].clientsingroup[i].client_socket_desc, client_output, strlen(client_output) + 1);
                                    }
                                }
                                groups[exittedgroup].clientsingroup[i].inGroup = false;
                            }
                        }
                    }
                    else
                    {
                        strcpy(client_output, "Server Message: you cant use this command if you are not in any group");
                        write(sock, client_output, strlen(client_output) + 1);
                    }
                    
                }
                else //undefined command control.
                {
                    strcpy(client_output, "Server Message: the command could not be understood.");
                    write(sock, client_output, strlen(client_output) + 1);
                }
            }
            else //one word input control.
            {
                strcpy(client_output, "Server Message: invalid input");
                write(sock, client_output, strlen(client_output) + 1);
            }
        }
    }
    free(socket_desc); //Free the socket pointer
    return 0;
}

int main(int argc, char *argv[])
{
    //initialize groupss
    //not using this caused send message error if there are 2 or more groups
    for (int i = 0; i < 20; i++)
    {
        groups[i].groupid = i;
    }

    //initiliaze server, socket variables
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(3205);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }

    listen(socket_desc, 3);

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0)
        {
            puts("Could not create thread");
            return 1;
        }
    }
    return 0;
}