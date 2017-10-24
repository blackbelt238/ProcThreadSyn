#include <stdio.h>
#include <stdlib.h>

/*
 * Simulates the Producer/Consumer problem.
 * Uses doubly-linked lists (though only singly-linked is necessary).
 */

// define a boolean datatype called "bool"
typedef enum {false, true} bool;

/** BUFFER LOGIC */

// buffer_node represents a node in a buffer
typedef struct buffer_n {
  int value; // less than 40
  struct buffer_n *next;
  struct buffer_n *prev;
} buffer_node;

// the buffer itself
typedef struct {
  buffer_node *head,
              *tail;
  int size; // max of 20
} buffer;

/* FUNCTION PROTOTYPES */
void add_to_buffer(buffer *buf, int value);
int remove_from_buffer(buffer *buf);

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

/** END BUFFER LOGIC */

/*void
consume() {

}

void
produce() {

}*/

int
main(void)
{
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
