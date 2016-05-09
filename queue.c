#include "queue.h"


/* QUEUE FUNCTIONS */
// Create a new empty queue
queue_t* queue_create() {
  queue_t* q = (queue_t*) malloc(sizeof(queue_t));
  if(q == NULL) {
    perror("Malloc");
    exit(EXIT_FAILURE);
  }
  q->head = NULL;
  q->tail = NULL;
  return q;
}


// Put an element at the end of a queue
void queue_put(queue_t* queue, const char* filename, uint32_t mask) {
  node_t* newNode = (node_t*) malloc(sizeof(node_t));
  if(newNode == NULL) {
    perror("Malloc");
    exit(EXIT_FAILURE);
  } 
  newNode->next = NULL;
  newNode->filename = filename;
  newNode->mask = mask;

  //check if queue is empty
  if(queue->head == NULL && queue->tail == NULL) {
    queue->head = queue->tail = newNode;
    return;
  }  
  queue->tail->next = newNode;
  queue->tail = newNode;
}


// Take an element off the front of a queue
node_t* queue_take(queue_t* queue) {
  node_t* node = queue->head;
  if (queue->head == NULL) {
    return NULL;
  }   
  else if (queue->head == queue->tail) {
    queue->head = queue->tail = NULL;
    return node;
  }
  else {
    queue->head = queue->head->next;
    return node;
  }
}

/* queue structs and functions taken from CSC-213 data structures lab also  */
/*  by Zoe Wolter and Thu Nyugen */
