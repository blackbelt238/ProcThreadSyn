#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Simulates the Producer/Consumer problem.
 * Uses doubly-linked lists (though only singly-linked is necessary).
 */

#define MAX_LOOP 50
#define MAX_BUFFER_SIZE 20
#define MAX_BUFFER_VALUE 39

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
void *init_buffer(void);
void destroy_buffer(buffer *buf);
void add_to_buffer(buffer *buf, int value);
int remove_from_buffer(buffer *buf);
void* consume1(void *argument);
void* produce1(void *argument);

/***** BUFFER LOGIC *****/
// init_buffer takes in a buffer and initializes all its values
void*
init_buffer(void) {
  buffer *buf;
  buf = (buffer*)malloc(sizeof(buffer));

  // initialize all vars
  buf->head = NULL;
  buf->tail = NULL;
  buf->size = 0;
  buf->mut = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(buf->mut, NULL);
  buf->not_empty = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(buf->not_empty, NULL);
  buf->not_full = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(buf->not_full, NULL);

  // add 3 vals to the buffer
  int i;
  for(i = 0; i < 3; i++){
    int new_val = rand() % 40;
    add_to_buffer(buf, new_val);
  }

  return (buf);
}

// destroy_buffer takes in a buffer and KILLS IT
void
destroy_buffer(buffer *buf) {
  // destroy contents of buffer
  int i;
  for(i = buf->size; i > 0; i--) {
    remove_from_buffer(buf);
  }

  // destroy base of buffer
  pthread_mutex_destroy(buf->mut);
  free(buf->mut);
  pthread_cond_destroy(buf->not_empty);
  free(buf->not_empty);
  pthread_cond_destroy(buf->not_full);
  free(buf->not_full);
  free(buf);
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
  if (buf->size == 1) { // if the last node was removed
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

  int i;
  for(i = 0; i < MAX_LOOP; i++) {
    pthread_mutex_lock(buf->mut);
    while(buf->size == 0) { // wait until the buffer gets some values
      printf("consumer1: BUFFER EMPTY\n");
      pthread_cond_wait(buf->not_empty, buf->mut);
    }

    // remove from buffer NOTE: add check for odd here (same as check for size above)
    int val = remove_from_buffer(buf);
    pthread_mutex_unlock(buf->mut);
    pthread_cond_signal(buf->not_full); // since we just removed, the buffer is no longer full
    printf("consumer1: removed %d\n", val);
  }

  return (NULL);
}

void*
produce1(void *argument) {
  buffer *buf = (buffer*) argument; // get the buffer that was passed in

  int i;
  for(i = 0; i < MAX_LOOP; i++) {
    pthread_mutex_lock(buf->mut);
    if(buf->size == MAX_BUFFER_SIZE) { // wait until the buffer is no longer full
      printf("producer1: BUFFER FULL\n");
      pthread_cond_wait(buf->not_full, buf->mut);
    }

    // add to buffer NOTE: add only an odd value
    int new_val = rand() % 40;
    add_to_buffer(buf, new_val);
    pthread_mutex_unlock(buf->mut);
    pthread_cond_signal(buf->not_empty); // since we just added, the buffer is no longer empty
    printf("producer1: added %d\n", new_val);
  }

  return (NULL);
}

int
main(void) {
  buffer *buf = init_buffer();
  pthread_t prod, cons;
  srand((unsigned int) time(NULL));

	// test threading from inital create to full destroy
  if (buf == NULL) {
    fprintf(stderr, "main: failed to initialize buffer\n");
    exit(1);
  }

  pthread_create(&prod, NULL, produce1, buf);
  pthread_create(&cons, NULL, consume1, buf);
  pthread_join(prod, NULL);
  pthread_join(cons, NULL);
  destroy_buffer(buf);

  printf("Success\n");

  return (0);
}
