#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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

//The following struct is able to store both type of datas (char* and int)
typedef struct IndexNode {
    DataStructure* data;
    Persona* record;
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


// Function needed to translate raw data types into a DataStructure, needed for pseudo-polymorphism, and used inside most other functions.
DataStructure* datafy(Dato dato, DataType type){
    DataStructure* data = malloc(sizeof(DataStructure));
    data->type = type;
    data->value = dato;
    return data;
}
// Compares the value of two DataStructures.
// The DataType must be the same, or else an error is thrown.
// If the first value is greater than the second, a value >0 is returned. Else, <0 is returned
int dataCompare(DataStructure* a, DataStructure* b){
    if (a->type != b->type){
        perror("Data Types do not match!");
    }
    if (a->type == STRING){
        return strcmp(a->value.stringa,b->value.stringa);
    }
    return a->value.numero > b->value.numero ? 1 : 0;
}
int dataCompareRaw(DataStructure* a, Dato b){
    if (a->type == STRING){
        return strcmp(a->value.stringa,b.stringa);
    }
    return a->value.numero > b.numero ? 1 : 0;
}
//Checks if the value of a DataStructure and of a raw Dato are the same.
int dataEquals(DataStructure* a, Dato b){
    if (a->type == STRING){
        return ( strcmp(a->value.stringa,b.stringa) == 0 );
    }
    return (a->value.numero == b.numero);
}

// Creates a node from a root_value, and the record (Persona) it is referencing
IndexNode* createNode(DataStructure* root_value, Persona* p){
    IndexNode* root = malloc(sizeof(IndexNode));
    if (root == NULL) {
        return NULL;
    }
    root->data = root_value;
    root->record = p;
    root->left = NULL;
    root->right = NULL;
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
        //printf("Creazione - %d\n", dato->value);
        //mio debug, non ci faccia caso che prima stava esplodendo tutto e volevo esplodere anche io
        return createNode(dato, p);
    }
    if (dataCompare(root->data, dato) > 0){ // If this is >0, it means that the value of the current node is greater, so our own is placed left
        //printf("Inserimento Sinistro- %d\n", dato->value);
        root->left = insertNode(root->left, dato, p);
        return root;
    }
    //printf("Inserimento Destro- %d\n", dato->value);
    root->right = insertNode(root->right, dato, p);
    return root;
}

void insert(Database * database, Persona * persona){ //Si, mi piace l'OOP! Come ha fatto a capirlo?!?!
    database->address = insertNode(database->address, datafy((Dato)persona->address, STRING), persona);
    database->age = insertNode(database->age, datafy((Dato)persona->age, INTEGER), persona);
    database->name = insertNode(database->name, datafy((Dato)persona->name, STRING), persona);
    database->surname = insertNode(database->surname, datafy((Dato)persona->surname, STRING), persona);
}

void inOrderVisit(IndexNode* node){
    if (node != NULL){
        inOrderVisit(node->left);
        printf("INORDERVISIT: %d\n", node->data->value);
        inOrderVisit(node->right);
    }
}

Persona* findByValue(IndexNode* node, Dato dato){
    if(node == NULL){
        return NULL;
    }
    if (dataEquals(node->data, dato)){
        return node->record;
    }
    if (dataCompareRaw(node->data, dato) > 0){
        return findByValue(node->left, dato);
    }
    return findByValue(node->right, dato);

}

Persona* findByName(Database * database, char * name){
    return findByValue(database->name, (Dato)name);
};
Persona* findBySurname(Database * database, char * surname){
    return findByValue(database->surname, (Dato)surname);
};
Persona* findByAddress(Database * database, char * address){
    return findByValue(database->address, (Dato)address);
};
Persona* findByAge(Database * database, int age){
    return findByValue(database->age, (Dato)age);
};


Persona p1 = {"Mario", "Rossi", "Mushroom Kingdom, 10", 23};
Persona p2 = {"Wario", "Gialli", "Mushroom Kingdom, 15", 22};
Persona p3 = {"Luigi", "Verdi", "Mushroom Kingdom, 12", 21};

int main(){
    Database* db = crateDatabase();
    //printf("%d", (&p1)->age);
    insert(db, &p1);
    insert(db, &p2);
    insert(db, &p3);
    inOrderVisit(db->age);
    printf("%s", findByAge(db, 21)->name);
    return 0;
};



