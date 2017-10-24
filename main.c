#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
 * Simulates the Producer/Consumer problem.
 * Uses doubly-linked lists (though only singly-linked is necessary).
 */

#define MAX_LOOP 50
#define MAX_BUFFER_SIZE 20
#define MAX_BUFFER_VALUE 39
srand((unsigned int)time(NULL));

// define a boolean datatype called "bool"
typedef enum {false, true} bool;

// buffer_node represents a node in a buffer
typedef struct buffer_n {
  int value;
  struct buffer_n *next;
  struct buffer_n *prev;
} buffer_node;

// the buffer itself
typedef struct {
  buffer_node *head,
              *tail;
  int size;

  // for threading
  pthread_mutex_t *mut;
  pthread_cond_t *not_empty,
                 *not_full;
} buffer;

/* FUNCTION PROTOTYPES */
void add_to_buffer(buffer *buf, int value);
int remove_from_buffer(buffer *buf);

/***** BUFFER LOGIC *****/
buffer
*new_buffer() {

}

void
destroy_buffer() {

}

// add a value to the buffer
void
add_to_buffer(buffer *buf, int value) {
  // create a new buffer node
  if (buf->size == 0) { // if the buffer is empty
    buf->tail = (buffer_node *)malloc(sizeof(buffer_node));
    buf->head = buf->tail;
  } else {              // if buffer is not empty
    buf->tail->next = (buffer_node *)malloc(sizeof(buffer_node)); // create the new node
    buf->tail->next->prev = buf->tail; // set the new node's previous node to the current end of the list
    buf->tail = buf->tail->next;
  }

  // place the value in the new buffer node
  buf->tail->value = value;
  buf->tail->next = NULL;
  ++(buf->size);
}

// removes the first node from the buffer, returning its value
int
remove_from_buffer(buffer *buf) {
  buffer_node *to_free;
  int to_return;

  // save the value of the node to be deleted
  to_free = buf->head;
  to_return = to_free->value;

  // delete the first node
  buf->head = to_free->next;
  if (buf->size == 0) { // if the last node was removed
    buf->tail = NULL;
  }
  else { // if there are nodes still in the list, ensure the head has no previous node
    buf->head->prev = NULL;
  }
  free(to_free);
  --(buf->size);

  return (to_return);
}
/***** END BUFFER LOGIC *****/

// deletes the first node if its value is odd
void*
consume1(void *argument) {
  buffer *buf = (buffer*) argument; // get the buffer that was passed in

  int i, val;
  for(i = 0; i < MAX_LOOP; i++) {
    pthread_mutex_lock(buf->mut);
    while(buf->length == 0) { // wait until the buffer gets some values
      printf("consumer1: buffer empty\n");
      pthread_cond_wait(buf->not_empty, buf->mut);
    }

    // remove from buffer NOTE: add check for odd here (same as check for length above)
    val = remove_from_buffer(buf);
    pthread_mutex_unlock(buf->mut);
    pthread_cond_signal(buf->not_full); // since we just removed, the buffer is no longer full
    printf("consumer1: removed %d\n", val);
  }
}

void*
produce1(void *argument) {
  buffer *buf = (buffer*) argument; // get the buffer that was passed in

  int i, new_val;
  for(i = 0; i < MAX_LOOP; i++) {
    pthread_mutex_lock(buf->mut);
    if(buf->length == MAX_BUFFER_SIZE) { // wait until the buffer is no longer full
      printf("producer1: buffer full\n");
      pthread_cond_wait(buf->not_full, buf->mut);
    }

    // add to buffer NOTE: add only an odd value
    new_val = rand() % 40;
    add_to_buffer(buf, new_val);
    pthread_mutex_unlock(buf->mut);
    pthread_cond_signal(buf->not_empty); // since we just added, the buffer is no longer empty
  }
}

int
main(void) {
	// test LinkedList operations by creating a buffer
  buffer *buf = &(buffer){NULL, NULL, 0};
  add_to_buffer(buf, 0); // add 0 to the buffer
  if (buf->size != 1) {
    printf("not added properly, len is %s\n", buf->size);
    return (1);
  }
  add_to_buffer(buf, 1);
  add_to_buffer(buf, 2);
  add_to_buffer(buf, 3);
  if (buf->size != 4) {
    printf("not added properly, len is %s\n", buf->size);
    return (1);
  }
  remove_from_buffer(buf);
  if (buf->size != 3 || buf->head->value != 1) {
    printf("not removed properly, len is %s, val is %s\n", buf->size, buf->head->value);
    return (1);
  }
  printf("Success\n");
}
