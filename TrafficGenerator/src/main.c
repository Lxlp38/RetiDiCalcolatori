#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#define PORT 9999
#define DURATION 20
#define NODEN 10

int id;
struct timeval start_time;
struct timeval current_time;
struct sockaddr_in server_addr;
struct sockaddr_in broadcast_addr;
int sock;

// typedef struct MessageNode{
//     int value;
//     struct MessageNode* next;
// } MessageNode;

// MessageNode* createMessageNode(MessageNode* root, int value){
//     if (root==NULL){
//         MessageNode* node = malloc(sizeof(MessageNode));
//         node->next = NULL;
//         node->value = value;
//         return node;
//     }
//     return createMessageNode(root->next, value);
// }

typedef struct NeighborNode{
    int id;
    int timestamp;
    struct NeighborNode* next;
} NeighborNode;

// ret is set to
// 1 if the new timestamp was greater
// 0 otherwise
NeighborNode* createNeighborNode(NeighborNode* root, int id, int value, int* ret){
    if (root==NULL){
        NeighborNode* node = malloc(sizeof(NeighborNode));
        node->id = id;
        node->timestamp = value;
        node->next = NULL;
        *ret = 1;
        return node;
    }
    if (root->id == id){
        *ret = 0;
        if ( value > root->timestamp ) {
            root->timestamp = value;
            *ret = 1;
        }
        return root;
    }
    root->next = createNeighborNode(root->next, id, value, ret);
    return root;
}

typedef struct Message{
    int id;
    int timestamp;
    int payload;
} Message;


// int searchHistoryMessages(MessageNode* root, int value){
//     if (root == NULL){
//         return 0;
//     }
//     if (root->value == value){
//         return 1;
//     }
//     return searchHistoryMessages(root->next,value);
// }

// Returns
// 0 if a node was found, but the timestamp was greater
// 1 if a node was found, and the timestamp was lower and thus updated
// 2 if no node was found
int graftHistory(NeighborNode* root, int id, int value){
    if (root==NULL){
        return 2;
    }
    if (root->id == id){
        if ( value > root->timestamp ) {
            root->timestamp = value;
            return 1;
        }
        return 0;
    }
    return graftHistory(root->next, id, value);
}


void readHistory(NeighborNode* root){
    if(root == NULL){
        printf("END\n");
        return;
    }
    printf("%d:%d |",root->id, root->timestamp);
    //readHistoryNode(root->messages);
    readHistory(root->next);
}

void killProcess(int signal){
    printf("\nProgram Finished\n");
    exit(0);
}

long getUTick(){
    gettimeofday(&current_time, NULL);
    return (current_time.tv_sec - start_time.tv_sec)*1000000 + (current_time.tv_usec - start_time.tv_usec);
    //return ( ( current_time.tv_sec - start_time.tv_sec ) - ( current_time.tv_usec - start_time.tv_usec )/1000000 );
}

int getTick(){
    return getUTick()/1000000;
}

int shouldStop(){
    return (getUTick() > DURATION*1000000) ? 1 : 0;
}


void printThroughputNode(int** nodevalues, int time, int node){
    int value = nodevalues[time][node];
    if (time>0){
        value += nodevalues[time-1][node];
    }
    printf("%d: ", node);
    if (value < 10){
        printf("%d", 0);
    }
    if (value < 100){
        printf("%d", 0);
    }
    printf("%d", value);
    if(node != NODEN-1){
        printf(", ");
    }
    else{
        printf("\n");
    }
    
    
    
}

void TrafficGenerator(int fildes[2], int signalstart[2]){
    printf("Traffic Generator Started: %d\n", getpid());
    close(fildes[0]);
    close(signalstart[1]);
    int timestamp = 0;

    if(id == 0){
        sleep(10);
        //Message message = {0, -2, 10};
        //write(fildes[1],(Message*)&message, sizeof(Message));
    }
    else{
        int ciao = 0;
        read(signalstart[0],&ciao,sizeof(int));
    }
    //Message startmessage = {0, -2, ciao};
    //write(fildes[1],(Message*)&startmessage,sizeof(Message));
    //sleep(ciao);
    //printf("%d", ciao);
    //sleep(ciao);

    for(int i = 0; i < (DURATION*1000000)/10000; i++){
        Message message = {id, timestamp, rand()};
        //printf("Sending %d %d %d to be broadcasted\n", message.id, message.timestamp, message.payload);
        write(fildes[1],(Message*)&message, sizeof(Message));
        timestamp++;
        usleep(10000);
    }
    Message message = {id, -1, 0};
    write(fildes[1],(Message*)&message, sizeof(Message));
    printf("Stopped: Traffic Generator\n");
    close(fildes[1]);
    close(signalstart[0]);
}

void Broadcaster(int fildes[2]){
    printf("Broadcaster Started: %d\n", getpid());
    close(fildes[1]);

    Message* message = calloc(1,sizeof(Message));

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("sock error\n");
        exit(1);
    }
    int optval = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval));
    if (ret == -1)
    {
        perror("setsockopt error\n");
        exit(0);
    }

    memset((void *)&broadcast_addr, 0, sizeof(struct sockaddr_in));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(PORT);

    while(1){
        memset(message, 0, sizeof(Message));
        read(fildes[0],message,sizeof(Message));
        if(message->timestamp == -1){
            printf("Broadcaster: received EOS");
            printf("\n");
            break;
        }
        //printf("Broadcasting %d %d %d message\n", message->id, message->timestamp, message->payload);
        sendto(sock, message, sizeof(Message), 0, (struct sockaddr*) &broadcast_addr, sizeof(struct sockaddr_in));
    }
    Message EOS = {id, -1, 0};
    sendto(sock, (struct Message*)&EOS, sizeof(Message), 0, (struct sockaddr*) &broadcast_addr, sizeof(struct sockaddr_in));
    close(sock);
    printf("Stopped: Broadcaster\n");
}

void TrafficReceiver(int fildes[2], int signalstart[2], int outputfds[2]){
    int started = 0;
    close(fildes[0]);
    close(signalstart[0]);
    close(outputfds[0]);
    //NeighborNode* history = calloc(1, sizeof(NeighborNode*));
    int* history = calloc(NODEN, sizeof(int));
    memset(history,-1,sizeof(int)*NODEN);
    Message* message = calloc(1, sizeof(Message));

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("sock error\n");
        exit(1);
    }
    int optval = 1;

    memset((void *)&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    int ret = bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        perror("bind error\n");
        exit(1);
    }

    while(1){
        memset(message, 0, sizeof(Message));
        int count = recvfrom(sock, message, sizeof(Message), 0, NULL, NULL);
        if (count < 0){
            perror("Could not receive message\n");
            continue;
        }
        if (started == 0){
            started = 1;
            write(signalstart[1],(int*)&message->payload,sizeof(int));
        }
        // if(started == 0 && message->id == 0 && message->timestamp == -2){
        //     started = 1;
        //     printf("Traffic Receiver: received start message\n");
        //     write(signalstart[1],(int*)&message->payload,sizeof(int));
        //     continue;
        // }
        if(message->id == id && message->timestamp == -1){
            printf("Traffic Receiver: received EOS\n");
            break;
        }

        if(history[message->id] < message->timestamp){
            //printf("Received %d %d %d\n", message->id, message->timestamp, message->payload);
            history[message->id] = message->timestamp;
            write(fildes[1], (int*)&message->id, sizeof(int));
            write(outputfds[1], message, sizeof(Message));
        }
        else{
            //printf("Received duplicate %d %d\n", message->id , message->timestamp);
        }
        
    }

    free(message);
    close(sock);
    int stop = -1;
    write(fildes[1], (int*)&stop, sizeof(int));
    close(fildes[1]);
    close(signalstart[1]);
    close(outputfds[1]);
    printf("Stopped: Traffic Receiver\n");
}

void TrafficAnalyzer(int fildes[2]){
    printf("Traffic Analyzer Started: %d\n", getpid());
    close(fildes[1]);

    int time = 0;
    int mypack = 0;
    int** nodevalues = calloc(DURATION, sizeof(int*));
    for(int i = 0; i < DURATION;i++){
        nodevalues[i] = calloc(NODEN, sizeof(int));
        for(int j = 0; j < NODEN; j++){
            nodevalues[i][j] = 0;
        }
    }

    while(1){
        int input;
        int check = 0;
        read(fildes[0], &input, sizeof(int));
        //printf("Traffic Analyzer: received %d\n", input);
        if( input == -1){
            //printf("Traffic Analizer: received EOS");
            break;
        }
        nodevalues[time][input]++;
        if (input == id){
            mypack++;
            if ((mypack+1)%100 == 0){
                for(int i = 0; i < NODEN; i++){
                    printThroughputNode(nodevalues, time, i);
                }
                time++;
                if (time >= DURATION-1){
                    break;
                }
                //printf("Traffic Analizer, time increased to %d\n", time+1);
            }
        }
    }
    for(int i = 0; i < NODEN; i++){
        printThroughputNode(nodevalues, time, i);
    }
    close(fildes[0]);
    printf("Stopped: Traffic Analyzer\n");
}


void outputSwitch(int outputfds[2], int signalstart[2]){
    printf("outputswitch triggered\n");
    int outputSwitch_pid = fork();
    switch(outputSwitch_pid){
        case -1:{
            perror("fork failed");
            return;
        }
        case 0:{
            TrafficGenerator(outputfds, signalstart);
            break;
        }
        default:{
            Broadcaster(outputfds);
            break;
        }
    }
}

void inputSwitch(int outputfds[2], int signalstart[2]){
    printf("inputswitch triggered\n");
    int fds[2];
    pipe(fds);
    int outputSwitch_pid = fork();
    switch(outputSwitch_pid){
        case -1:{
            perror("fork failed");
            return;
        }
        case 0:{
            TrafficReceiver(fds, signalstart, outputfds);
            break;
        }
        default:{
            TrafficAnalyzer(fds);
            break;
        }
    }
}


int main(int argc, char *argv[]) {

    id = atoi(argv[1]);
    int opid = getpid();

    int signalstart[2];
    pipe(signalstart);

    int outputfds[2];
    pipe(outputfds);

    // Autoflush stdout for docker
    setvbuf(stdout, NULL, _IONBF, 0);

    gettimeofday(&start_time, NULL);

    // sock = socket(AF_INET, SOCK_DGRAM, 0);
    // if (sock < 0)
    // {
    //     perror("sock error\n");
    //     exit(1);
    // }
    // int optval = 1;
    // int ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval));
    // if (ret == -1)
    // {
    //     perror("setsockopt error\n");
    //     exit(0);
    // }

    // memset((void *)&server_addr, 0, sizeof(struct sockaddr_in));
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // server_addr.sin_port = htons(PORT);

    // memset((void *)&broadcast_addr, 0, sizeof(struct sockaddr_in));
    // broadcast_addr.sin_family = AF_INET;
    // broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    // broadcast_addr.sin_port = htons(PORT);


    // ret = bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    // if (ret < 0)
    // {
    //     perror("bind error\n");
    //     exit(1);
    // }

    int child_pid = fork();
    switch(child_pid){
        case -1:{
            perror("fork failed");
            return 1;
        }
        case 0:{
            outputSwitch(outputfds, signalstart);
            break;
        }
        default:{
            inputSwitch(outputfds, signalstart);
            break;
        }
    }
    
    // if(opid == getpid()){
    //     while(shouldStop() != 1){
    //         sleep(1);
    //     }
    //     sleep(1);
    //     printf("socket closed\n");
    //     close(sock);
    // }
}