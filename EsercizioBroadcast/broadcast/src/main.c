#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <ctype.h>
# include <signal.h>

#define IP_FOUND "IP_FOUND"
#define IP_FOUND_ACK "IP_FOUND_ACK"
#define PORT 9999
#define PORT_BROADCAST 9998
#define BUFFSIZE 1024
#define MESSAGESIZE 2000

int id;

typedef struct MessageNode{
    int value;
    struct MessageNode* next;
} MessageNode;

MessageNode* createMessageNode(MessageNode* root, int value){
    if (root==NULL){
        MessageNode* node = malloc(sizeof(MessageNode));
        node->next = NULL;
        node->value = value;
        return node;
    }
    return createMessageNode(root->next, value);
}


typedef struct NeighborNode{
    int id;
    struct NeighborNode* next;
    struct MessageNode* messages;
} NeighborNode;


NeighborNode* createNeighborNode(NeighborNode* root, int id, int value){
    if (root==NULL){
        NeighborNode* node = malloc(sizeof(NeighborNode));
        node->id = id;
        node->next = NULL;
        node->messages = createMessageNode(NULL,value);
        return node;
    }
    if (root->id == id){
        root->messages = createMessageNode(root->messages,value);
        return root;
    }
    return createNeighborNode(root->next, id, value);
}

typedef struct Message{
    int oid;
    int timestamp;
    int lastid;
    int payload;
} Message;

// typedef union NodeType{
//     NeighborNode* neighbor;
//     MessageNode* message;
// } NodeType;

int searchHistoryMessages(MessageNode* root, int value){
    if (root == NULL){
        return 0;
    }
    if (root->value == value){
        return 1;
    }
    return searchHistoryMessages(root->next,value);
}

int searchHistory(NeighborNode* root, int id, int value){
    if (root==NULL){
        return 0;
    }
    if (root->id == id){
        return searchHistoryMessages(root->messages,value);
    }
    return searchHistory(root->next, id, value);
}

void readHistoryNode(MessageNode* root){
    if (root == NULL){
        printf(" --> END\n");
        return;
    }
    printf(" --> %d", root->value);
    readHistoryNode(root->next);
}

void readHistory(NeighborNode* root){
    if(root == NULL){
        printf("END\n");
        return;
    }
    printf("%d |",root->id);
    readHistoryNode(root->messages);
    readHistory(root->next);
}

void logger(char * format, ...) {
    va_list args;
    va_start(args, format);
    printf("%d: ", id);
    printf(format, args);
    printf("\n");
    va_end(args);
}

int valueinarray(int val, int* arr, int size){
    for(int i = 0; i < size; i++){
        if(arr[i] == val) return 1;
    }
    return 0;
}

void formatMessage(char* buffer, int oid, int id, int timestamp){
    memset(buffer, '\0', BUFFSIZE);
    snprintf(buffer,BUFFSIZE,"%d|%d|%d",oid,timestamp,id);
    printf("formatMessage: %s\n", buffer);
};

void readMessage(char* message, int* oid, int* timestamp, int* lid){
    char* token = strtok(message, "|");
    *oid = atoi(token);
    token = strtok(NULL, "|");
    *timestamp = atoi(token);
    token = strtok(NULL, "|");
    *lid = atoi(token);
    printf("readMessage: oid %d, timestamp %d, lid %d\n", *oid, *timestamp, *lid);
}

void killProcess(int signal){
    printf("Program Finished\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    int sock, input_sock, input_len; // broadcast_sock;
    int optval = 1;
    struct sockaddr_in broadcast_addr;
    struct sockaddr_in server_addr;
    int addr_len = sizeof(struct sockaddr_in);
    int ret, count;
    char buffer[BUFFSIZE];
    char tempbuffer[BUFFSIZE];
    NeighborNode* history = NULL;
    int timestamp = 0;
    int oid, otimestamp, lid;
    Message* message;

    memset(buffer, '\0', BUFFSIZE);
    memset(tempbuffer, '\0', BUFFSIZE);

    id = atoi(argv[1]);

    printf("Inizializing...\n");

    if (argc < 3) {
        perror("No neighbor for this node\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("sock error\n");
        return -1;
    }

    // broadcast_sock = socket(AF_INET, SOCK_DGRAM, 0);
    // if (broadcast_sock < 0)
    // {
    //     perror("broadcast_sock error");
    //     return -1;
    // }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval));
    if (ret == -1)
    {
        perror("setsockopt error\n");
        return 0;
    }


    int * neighbors = calloc(sizeof(int), argc - 2);
    for (int i = 0; i < argc - 2; i ++) {
        neighbors[i] = atoi(argv[i + 2]);
    }


    memset((void *)&server_addr, 0, addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    memset((void *)&broadcast_addr, 0, addr_len);
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(PORT);


    ret = bind(sock, (struct sockaddr *)&server_addr, addr_len);
    if (ret < 0)
    {
        perror("bind error\n");
        return -1;
    }

    printf("Done with binding\n");


    // Implementation

    printf("Server started\n");

    signal(SIGALRM,killProcess);
    alarm(5);

    if(id==0){
        sleep(1);
        //formatMessage(buffer,id,id,timestamp);
        Message newmessage = {id, timestamp, id, 0};
        printf("Init: Sending %d %d %d message back to broadcast\n", newmessage.oid, newmessage.timestamp, newmessage.lastid);
        sendto(sock, (Message*)&newmessage, sizeof(Message), 0, (struct sockaddr*) &broadcast_addr, addr_len);
        timestamp++;
    }

    while (1)
    {
        // lid = 0;
        // oid = 0;
        // otimestamp = 0;
        memset(buffer, '\0', BUFFSIZE);
        memset(tempbuffer, '\0', BUFFSIZE);
        message = calloc(1, sizeof(Message));
        count = recvfrom(sock, message, sizeof(Message), 0, NULL, NULL);
        if (count < 0){
            perror("Could not receive message\n");
            continue;
        }
        printf("History is being read!\n");
        readHistory(history);


        //printf("Message lenght: %d, Buffer is: %s\n", count, buffer);
        //strcpy(tempbuffer,buffer);
        //readMessage(tempbuffer,&oid,&otimestamp,&lid);
        printf("Client connection information:\t Id: %d\ttimestamp: %d\tlid: %d\n", message->oid, message->timestamp, message->lastid);

        if(valueinarray(message->lastid,neighbors,argc-2) == 0){
            printf("Node %d is not a neighbor, skipping.\n", message->lastid);
            continue;
        }

        if(message->oid == id || searchHistory(history,message->oid,message->timestamp) == 1){
            printf("Node %d sent a duplicate message: %d. Skipping\n", message->lastid, message->payload);
            continue;
        }
        printf("Input node %d is a neighbor\n", message->lastid);
        history = createNeighborNode(history,message->oid,message->timestamp);
        //formatMessage(buffer,oid,id,otimestamp);
        Message newmessage = {message->oid, message->timestamp, id, 0};
        free(message);
        printf("Sending %d message back to broadcast\n", newmessage.payload);
        sendto(sock, (Message*)&newmessage, sizeof(Message), 0, (struct sockaddr *)&broadcast_addr, addr_len);
    }

    // End
    close(sock);
    free(neighbors);

}