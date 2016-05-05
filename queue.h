#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>



// Define a struct for queue nodes to track events
typedef struct node {
  uint32_t mask;
  const char* filename;
  struct node* next;
} node_t;
  
typedef struct queue {
  node_t* head;
  node_t* tail;
} queue_t;


// Queue header file 
queue_t* queue_create();
void queue_put(queue_t* queue, const char* filename, uint32_t mask);
node_t* queue_take(queue_t* queue);

#endif
