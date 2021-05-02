#include "pa3.h"

typedef struct Node {
    char* key;
    char* value;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
    pthread_mutex_t llLock;
} LinkedList;

LinkedList* initLL(){
    LinkedList* ll = (LinkedList*) malloc(sizeof(LinkedList));
    if( ll == NULL ){
        perror("Malloc Failed in initLL");
        exit(EXIT_FAILURE);
    }
    ll->head = NULL;
    //pthread_mutex_init(&ll->llLock, NULL);
    return ll;
}

// Do not free the key or value char* after calling this -- use destroyLL to free the whole LL
bool addNode(LinkedList* ll, char* key, char* value){
    if( ll == NULL ){
        if(DEBUG) fprintf(stderr, "%s", "addNode Failed: LL Not Initialized");
        return false;
    }

    Node* temp = (Node*) malloc(sizeof(Node));
    if( temp == NULL ){
        perror("Malloc Failed in addNode");
        exit(EXIT_FAILURE);
    }
    temp->key = key;
    temp->value = value;
    temp->next = NULL;

    //pthread_mutex_lock(&ll->llLock);
    if(ll->head == NULL){
        //printf("Emptry Head Not Anymore: %s\n", key);
        ll->head = temp;
    }else{
       // printf("Head Full : ");
        Node* prev = NULL;
        Node* ptr = ll->head;
        printf("%s\n", ptr->key);
        while(ptr != NULL){
            /*if(strcmp(key, ptr->key) <= 0){
                if( prev == NULL ){
                    ll->head = temp;
                    temp->next = ptr;
                }else{
                    prev->next = temp;
                    temp->next = ptr;
                }

                pthread_mutex_unlock(&ll->llLock);
                return true;
            }*/
            prev = ptr;
            ptr = ptr->next;
        }
        if( prev == NULL ){
            ll->head = temp;
            temp->next = ptr;
        }else{
            prev->next = temp;
            temp->next = ptr;
        }
    }
    pthread_mutex_unlock(&ll->llLock);
    return false;
}

// Do not free newValue char* after calling this -- use destroyLL to free the whole LL
bool changeNodeValue(LinkedList* ll, char* key, char* newValue){
    if( ll == NULL ){
        if(DEBUG) fprintf(stderr, "%s", "changeNodeValue Failed: LL Not Initialized");
        return false;
    }
    pthread_mutex_lock(&ll->llLock);
    Node* ptr = ll->head;
    while(ptr != NULL){
        if(strcmp(key, ptr->key) == 0){
            char* temp = ptr->value;
            ptr->value = newValue;
            free(temp);
            pthread_mutex_unlock(&ll->llLock);
            return true;
        }
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&ll->llLock);
    return false;
}

// Do not free the return char* from the method call  -- use destroyLL to free the whole LL
char* getValueAtKey(LinkedList* ll, char* key){
    if( ll == NULL ){
        if(DEBUG) fprintf(stderr, "%s", "changeNodeValue Failed: LL Not Initialized");
        return NULL;
    }
    pthread_mutex_lock(&ll->llLock);
    Node* ptr = ll->head;
    while(ptr != NULL){
        if(strcmp(key, ptr->key) == 0){
            char* temp = ptr->value;
            pthread_mutex_unlock(&ll->llLock);
            return temp;
        }
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&ll->llLock);
    return NULL;
}

// You have to free the returned char* from this call.
char* deleteKey(LinkedList* ll, char* key){
    if(ll == NULL){
        if(DEBUG) fprintf(stderr, "%s", "Destroy LL Failed: LL Not Initialized");
        return NULL;
    }
   pthread_mutex_lock(&ll->llLock);
    Node* prev = NULL;
    Node* ptr = ll->head;
    while(ptr != NULL){
        if( strcmp(ptr->key, key) == 0){
            if(prev == NULL){
                ll->head = ll->head->next;
                free(ptr->key);
                char* temp = ptr->value;
                free(ptr);
                pthread_mutex_unlock(&ll->llLock);
                return temp;
            }else{
                prev->next = ptr->next;
                free(ptr->key);
                char* temp = ptr->value;
                free(ptr);
                pthread_mutex_unlock(&ll->llLock);
                return temp;
            }
        }
        prev = ptr;
        ptr = ptr->next;
    }
   pthread_mutex_unlock(&ll->llLock);
    return NULL;
}

bool destroyLL(LinkedList* ll){
    if(ll == NULL){
        if(DEBUG) fprintf(stderr, "%s", "Destroy LL Failed: LL Not Initialized");
        return false;
    }
    pthread_mutex_lock(&ll->llLock);
    Node* ptr = ll->head;
    while(ptr != NULL){
        Node* next = ptr->next;
        free(ptr->key);
        free(ptr->value);
        free(ptr);
        ptr = next;
    }
    pthread_mutex_unlock(&ll->llLock);
    pthread_mutex_destroy(&ll->llLock);
    free(ll);
    return true;
}

void printLL(LinkedList* ll){
    if( ll == NULL ){
        if(DEBUG) fprintf(stderr, "%s", "printLL Failed: LL Not Initialized");
        return;
    }
    pthread_mutex_lock(&ll->llLock);
    Node* ptr = ll->head;
    printf("LL:\t");
    while(ptr != NULL){
        if(ptr->next == NULL) printf("(key: %s, value: %s)", ptr->key, ptr->value);
        else printf("(key: %s, value: %s) ---> ", ptr->key, ptr->value);
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&ll->llLock);
}