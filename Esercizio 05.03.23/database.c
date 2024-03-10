#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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

typedef struct Persona {
    char name[20];
    char surname[50];
    char address[100];
    int age;
} Persona;

// Constructs needed to achieve pseudo-polymorphism in a somewhat satisfactory way
typedef enum DataType{
    INTEGER,
    STRING
} DataType;
typedef union Dato{
    int numero;
    char* stringa;
} Dato;
typedef struct DataStructure {
    DataType type;
    Dato value;
} DataStructure;

typedef struct PersonaNode {
    Persona* persona;
    struct PersonaNode* next;
} PersonaNode;

// The following struct is able to store both type of datas (char* and int)
// By also storing the related record, it is possible to fetch it with more simplicity
typedef struct IndexNode {
    DataStructure* data;
    PersonaNode* records;
    struct IndexNode* left;
    struct IndexNode* right;
} IndexNode;

typedef struct Database{
    IndexNode* name;
    IndexNode* surname;
    IndexNode* address;
    IndexNode* age;
} Database;


void insert(Database * database, Persona * persona);
Persona* findByName(Database * database, char * name);
Persona* findBySurname(Database * database, char * surname);
Persona* findByAddress(Database * database, char * address);
Persona* findByAge(Database * database, int age);

////


/***
 *      _____        _         _____ _                   _                     ____                       _   _                 
 *     |  __ \      | |       / ____| |                 | |                   / __ \                     | | (_)                
 *     | |  | | __ _| |_ __ _| (___ | |_ _ __ _   _  ___| |_ _   _ _ __ ___  | |  | |_ __   ___ _ __ __ _| |_ _  ___  _ __  ___ 
 *     | |  | |/ _` | __/ _` |\___ \| __| '__| | | |/ __| __| | | | '__/ _ \ | |  | | '_ \ / _ \ '__/ _` | __| |/ _ \| '_ \/ __|
 *     | |__| | (_| | || (_| |____) | |_| |  | |_| | (__| |_| |_| | | |  __/ | |__| | |_) |  __/ | | (_| | |_| | (_) | | | \__ \
 *     |_____/ \__,_|\__\__,_|_____/ \__|_|   \__,_|\___|\__|\__,_|_|  \___|  \____/| .__/ \___|_|  \__,_|\__|_|\___/|_| |_|___/
 *                                                                                  | |                                         
 *                                                                                  |_|                                         
 */
// Function needed to translate raw data types into a DataStructure, needed for pseudo-polymorphism, and used inside most other functions.
DataStructure* datafy(Dato dato, DataType type){
    DataStructure* data = malloc(sizeof(DataStructure));
    data->type = type;
    data->value = dato;
    return data;
}
// Compares the value of two DataStructures.
// The DataType must be the same, or else an error is thrown.
// If the first value is greater than the second, a value >0 is returned. Else, <0 is returned. If the data are equals, 0 is returned.
int dataCompare(DataStructure* a, Dato b){
    if (a->type == STRING){
        return strcmp(a->value.stringa,b.stringa);
    }
    return a->value.numero > b.numero ? 1 : ( a->value.numero == b.numero ) ? 0 : -1;
}



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


PersonaNode* createPersonaNode(Persona* p){
    PersonaNode* pn = malloc(sizeof(PersonaNode));
    pn->persona = p;
    pn->next = NULL;
    return pn;
}

// Creates a node from a root_value, and the record (Persona) it is referencing
IndexNode* createNode(DataStructure* root_value, Persona* p){
    IndexNode* root = malloc(sizeof(IndexNode));
    if (root == NULL) {
        return NULL;
    }
    PersonaNode* personanode = createPersonaNode(p);
    root->data = root_value;
    root->records = personanode;
    root->left = NULL;
    root->right = NULL;
    return root;
}


PersonaNode* addPersonaNode(PersonaNode* root, Persona* p){
    if (root == NULL){
        return createPersonaNode(p);
    }
    root->next = addPersonaNode(root->next,p);
    return root;
}

// Initializes a Database at NULL values
Database * crateDatabase(){
    Database * db = malloc(sizeof(Database));
    db->address = NULL;
    db->age = NULL;
    db->name = NULL;
    db->surname = NULL;
    return db;
}

// Inserts a node in a orderly manner.
IndexNode* insertNode(IndexNode* root, DataStructure* dato, Persona* p){
    if (root == NULL){
        return createNode(dato, p);
    }
    if (dataCompare(root->data, dato->value) > 0){
        root->left = insertNode(root->left, dato, p);
        return root;
    }
    if (dataCompare(root->data, dato->value) == 0){
        root->records = addPersonaNode(root->records, p);
        // since the Persona has simply been added to the records LinkedList without the need to create another IndexNode
        // the DataStructure is not needed and can be freed
        free(dato);
        return root;
    }
    root->right = insertNode(root->right, dato, p);
    return root;
}

void insert(Database * database, Persona * persona){
    database->address = insertNode(database->address, datafy((Dato)persona->address, STRING), persona);
    database->age = insertNode(database->age, datafy((Dato)persona->age, INTEGER), persona);
    database->name = insertNode(database->name, datafy((Dato)persona->name, STRING), persona);
    database->surname = insertNode(database->surname, datafy((Dato)persona->surname, STRING), persona);
}


/***
 *      __  __                                   ______             _             
 *     |  \/  |                                 |  ____|           (_)            
 *     | \  / | ___ _ __ ___   ___  _ __ _   _  | |__ _ __ ___  ___ _ _ __   __ _ 
 *     | |\/| |/ _ \ '_ ` _ \ / _ \| '__| | | | |  __| '__/ _ \/ _ \ | '_ \ / _` |
 *     | |  | |  __/ | | | | | (_) | |  | |_| | | |  | | |  __/  __/ | | | | (_| |
 *     |_|  |_|\___|_| |_| |_|\___/|_|   \__, | |_|  |_|  \___|\___|_|_| |_|\__, |
 *                                        __/ |                              __/ |
 *                                       |___/                              |___/ 
 */

void freePersonaNode(PersonaNode* node){
    if (node == NULL){
        return;
    }
    freePersonaNode(node->next);
    free(node);
}

void freeNodes(IndexNode* node){
    if (node == NULL){
        return;
    }
    freeNodes(node->left);
    freeNodes(node->right);
    freePersonaNode(node->records);
    free(node->data);
    free(node);
}

void freeDatabase(Database* database){
    freeNodes(database->address);
    freeNodes(database->age);
    freeNodes(database->name);
    freeNodes(database->surname);
    free(database);
}



/***
 *      _____                        _       ______ _           _ _             
 *     |  __ \                      | |     |  ____(_)         | (_)            
 *     | |__) |___  ___ ___  _ __ __| |___  | |__   _ _ __   __| |_ _ __   __ _ 
 *     |  _  // _ \/ __/ _ \| '__/ _` / __| |  __| | | '_ \ / _` | | '_ \ / _` |
 *     | | \ \  __/ (_| (_) | | | (_| \__ \ | |    | | | | | (_| | | | | | (_| |
 *     |_|  \_\___|\___\___/|_|  \__,_|___/ |_|    |_|_| |_|\__,_|_|_| |_|\__, |
 *                                                                         __/ |
 *                                                                        |___/ 
 */
PersonaNode* findByValue(IndexNode* node, Dato dato){
    if(node == NULL){
        return NULL;
    }
    if (dataCompare(node->data, dato) == 0){
        return node->records;
    }
    if (dataCompare(node->data, dato) > 0){
        return findByValue(node->left, dato);
    }
    return findByValue(node->right, dato);

}

// Please note: each of those functions will return only the first Persona to match in order to maintain compatibility
Persona* findByName(Database * database, char * name){
    return findByValue(database->name, (Dato)name)->persona;
};
Persona* findBySurname(Database * database, char * surname){
    return findByValue(database->surname, (Dato)surname)->persona;
};
Persona* findByAddress(Database * database, char * address){
    return findByValue(database->address, (Dato)address)->persona;
};
Persona* findByAge(Database * database, int age){
    return findByValue(database->age, (Dato)age)->persona;
};






/***
 *      _____       _                 
 *     |  __ \     | |                
 *     | |  | | ___| |__  _   _  __ _ 
 *     | |  | |/ _ \ '_ \| | | |/ _` |
 *     | |__| |  __/ |_) | |_| | (_| |
 *     |_____/ \___|_.__/ \__,_|\__, |
 *                               __/ |
 *                              |___/ 
 */
void inOrderVisit(IndexNode* node){
    if (node != NULL){
        inOrderVisit(node->left);
        printf("INORDERVISIT: %s\n", node->data->value);
        inOrderVisit(node->right);
    }
}

Persona p1 = {"Mario", "Rossi", "Mushroom Kingdom, 10", 23};
Persona p4 = {"Mario2LaVendetta", "Rossi", "Mushroom Kingdom, 10", 23};
Persona p2 = {"Wario", "Gialli", "Mushroom Kingdom, 15", 22};
Persona p3 = {"Luigi", "Verdi", "Mushroom Kingdom, 12", 21};

int main(){
    Database* db = crateDatabase();
    //printf("%d", (&p1)->age);
    insert(db, &p1);
    insert(db, &p2);
    insert(db, &p3);
    insert(db, &p4);
    inOrderVisit(db->surname);
    printf("%s", findByValue(db->age, (Dato)23)->next->persona->name);
    freeDatabase(db);
    //inOrderVisit(db->surname);
    return 0;
};



