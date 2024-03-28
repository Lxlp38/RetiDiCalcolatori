#include <signal.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

#define SIZE 100
const size_t k_max_msg = 20000;

/***
 *      _____        _          _______                    
 *     |  __ \      | |        |__   __|                   
 *     | |  | | __ _| |_ __ _     | |_   _ _ __   ___  ___ 
 *     | |  | |/ _` | __/ _` |    | | | | | '_ \ / _ \/ __|
 *     | |__| | (_| | || (_| |    | | |_| | |_) |  __/\__ \
 *     |_____/ \__,_|\__\__,_|    |_|\__, | .__/ \___||___/
 *                                    __/ | |              
 *                                   |___/|_|              
 */

typedef struct Node{
    char* key;

    char* value;
    time_t expiration;

    struct Node* next;
} Node;

typedef struct HashMap{
    Node* table[SIZE];
} HashMap;

HashMap* map;
sem_t hash_sems[SIZE];

/***
 *      _   _           _             _    _                 _ _ _             
 *     | \ | |         | |           | |  | |               | | (_)            
 *     |  \| | ___   __| | ___  ___  | |__| | __ _ _ __   __| | |_ _ __   __ _ 
 *     | . ` |/ _ \ / _` |/ _ \/ __| |  __  |/ _` | '_ \ / _` | | | '_ \ / _` |
 *     | |\  | (_) | (_| |  __/\__ \ | |  | | (_| | | | | (_| | | | | | | (_| |
 *     |_| \_|\___/ \__,_|\___||___/ |_|  |_|\__,_|_| |_|\__,_|_|_|_| |_|\__, |
 *                                                                        __/ |
 *                                                                       |___/ 
 */

Node* createNode(char* key, char* value, long expiration){
    Node* node = calloc(1,sizeof(Node));
    node->key = strdup(key);
    node->value = strdup(value);
    if (expiration == -1){
        node->expiration = -1;
    } else{
        node->expiration = time(NULL) + expiration;
    }
    return node;
    
}

HashMap* createHashMap(){
    HashMap* map_temp = calloc(1, sizeof(HashMap));
    //map->table = calloc(size, sizeof(Node*));
    for(int i = 0; i<SIZE; i++){
        map_temp->table[i] = NULL;
    }
    return map_temp;
}

int hash(char* key) {
    int hash = 0;
    for (int i = 0; key[i] != '\0'; ++i)
        hash = (hash * 31 + key[i]) % SIZE;
    return hash;
}

Node* insertNode(Node* root, Node* newNode){
    if(root == NULL){
        return newNode;
    }
    if(strcmp(root->key, newNode->key) == 0){
        newNode->next = root->next;
        return newNode;
    }
    return root;
}

void insert(struct HashMap* hm, char* key, char* value, long expiration) {
    int index = hash(key);
    sem_wait(&hash_sems[index]);
    struct Node* newNode = createNode(key, value, expiration);
    //printf("%s %d added\n", key, index);
    if (hm->table[index] == NULL) {
        hm->table[index] = newNode;
        //printf("Added at front\n");
        sem_post(&hash_sems[index]);
        return;
    }
    hm->table[index] = insertNode(hm->table[index], newNode);
    sem_post(&hash_sems[index]);
}

char* search(struct HashMap* hm, char* key) {
    int index = hash(key);
    //printf("before wait");
    sem_wait(&hash_sems[index]);
    //printf("after wait");
    struct Node* temp = hm->table[index];
    //printf("%s %d searching\n", key, index);
    while (temp != NULL) {
        if (strcmp(temp->key, key) == 0){
            //printf("trovato");
            if ((temp->expiration == -1 || temp->expiration > time(NULL))){
                sem_post(&hash_sems[index]);
                //printf("here");
                return temp->value;
            }
            sem_post(&hash_sems[index]);
            return "null";
        }
        temp = temp->next;
    }
    sem_post(&hash_sems[index]);
    return "null";
}


void exit_with_error(const char * msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}



char* redis_command_client(char* token_pointer){
    //char* token = strtok_r(NULL, "\r\n", &token_pointer);
    return "+OK\r\n";
    //return "-Error\r\n";
}

char* redis_command_set(char* token_pointer){
    char *token = strtok_r(NULL, "\r\n", &token_pointer);
    token = strtok_r(NULL, "\r\n", &token_pointer);
    char* key = strdup(token);
    strtok_r(NULL, "\r\n", &token_pointer);
    token = strtok_r(NULL, "\r\n", &token_pointer);
    char* value = strdup(token);
    int ex = -1;
    strtok_r(NULL, "\r\n", &token_pointer);
    while(token != NULL){
        if (strcmp(token, "EX") == 0){
            strtok_r(NULL, "\r\n", &token_pointer);
            token = strtok_r(NULL, "\r\n", &token_pointer);
            ex = atoi(token);
            break;
        }
        token = strtok_r(NULL, "\r\n", &token_pointer);
    }
    printf("INSERT %s %s %d", key, value, ex);
    insert(map,key,value,ex);
    return "+OK\r\n";
}

char* redis_command_get(char* token_pointer){
    char *token = strtok_r(NULL, "\r\n", &token_pointer);
    token = strtok_r(NULL, "\r\n", &token_pointer);
    char* ret = strdup(search(map, token));
    //printf(" HELLOWORLD %s\n", ret);
    if (strcmp(ret, "null") == 0){
        return "$-1\r\n";
    }
    char temp[1024];
    //printf("$%d|%s\n", (int)strlen(ret), ret);
    sprintf(temp, "$%d\r\n%s\r\n", (int)strlen(ret), ret);
    return strdup(temp);
}

char* parse_redis_request(char *request) {
    char *token_pointer; 
    char *token = strtok_r(request, "\r\n", &token_pointer);
    while (token != NULL) {
        printf("Argument: %s\t", token);
        if (strncmp(token,"*",1) == 0 || strncmp(token,"$",1) == 0){
            //printf("ops");
            token = strtok_r(NULL, "\r\n", &token_pointer);
            continue;
        }
        if (strcmp(token, "CLIENT") == 0){
            //printf("client");
            return redis_command_client(token_pointer);
        }
        if (strcmp(token, "GET") == 0){
            //printf("get");
            return redis_command_get(token_pointer);
        }
        if (strcmp(token, "SET") == 0){
            //printf("get");
            return redis_command_set(token_pointer);
        }
        //printf("next!");
        token = strtok_r(NULL, "\r\n", &token_pointer);
    }
    return "-Error\r\n";
}

void replace_char(char *str, char target, char replacement) {
    // Iterate through the string
    for (int i = 0; str[i] != '\0'; i++) {
        // If the current character matches the target, replace it
        if (str[i] == target) {
            str[i] = replacement;
        }
    }
}

void* connection_handling(void* arg){
    signal(SIGPIPE, SIG_IGN);
    int connfd = *(int*) arg;
    while(1){
        char rbuf[1024];
        ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
        if (n < 0) {
            printf("read() error");
            break;
        }
        printf("\n");
        char* answer = strdup(parse_redis_request(rbuf));
        if (write(connfd, answer, strlen(answer)) == -1){
            break;
        };
    }
    close(connfd);
    pthread_exit(NULL);
}

void sigpipe_handler(int signum) {
    // Simply ignore the signal
    printf("Ignoring SIGPIPE signal\n");
}

int main() {

    setvbuf(stdout, NULL, _IONBF, 0);

    map = createHashMap();

    struct sigaction sa;
    //struct sigaction sa = { 0 };
    // Initialize the sigaction struct
    sa.sa_handler = sigpipe_handler; // Set the signal handler function
    sigemptyset(&sa.sa_mask);        // Clear the signal mask
    sa.sa_flags = 0;                 // Set the flags to 0

    // Install the signal handler for SIGPIPE
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i<SIZE; i++){
        sem_init(&hash_sems[i],0,1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        exit_with_error("sock error\n");
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr;
    memset((void *)&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(7379);
    addr.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    int rv = bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (rv) {
        exit_with_error("bind()");
    }
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        exit_with_error("listen()");
    }

    // Create a process for each connection to serve set and get requested
    while (1) {
        // accept
        printf("1111111111111111111");
        struct sockaddr_in client_addr;
        memset((void *)&client_addr, 0, sizeof(struct sockaddr_in));
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue;   // error
        }
        printf("3333333333333333333");
        pthread_t id;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&id, &attr, connection_handling, (void *)&connfd);
    }
    printf("22222222222");


}

