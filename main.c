#include "io.h"
#include "mm.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

char msg[] = "\n ** You need to copy your main.c file from Assignment 1 **\n";


//-------------------------------------------------------

int read_char() {
  ssize_t result;
  char c;
  result = read(STDIN_FILENO, &c, 1);
  if (result == 1) {
    return (int)c;
  }
  return EOF;
}

int write_char(char c) {
  ssize_t result = write(STDOUT_FILENO, &c, 1);
  if (result == 1) {
    return 0;
  }
  return EOF;
}

int write_string(char* s) {
  if(s == NULL) {
    errno = EINVAL;
    return EOF;
  }
  for(int i = 0; s[i] != '\0'; i++) {
    if(write_char(s[i]) == EOF) {
      return EOF;
    }
  }
  return EOF;
}

int write_int(int n) {
  char buffer[12]; // Enough to hold -2147483648 and null terminator 
  int is_negative = 0;
  int i = 0;
  int j = 0;
  if(n<0){
    is_negative = 1;
    n = -n; // Make n positive
  }
  if(n==0){
    buffer[i++] = '0';
  }else{
    while(n>0){
      buffer[i++] = (n%10) + '0'; // Convert digit to character
      n = n/10;
    }
  }

  if(is_negative){
    buffer[i++] = '-'; // Add negative sign
  }

  // Reverse the string
  for(j = 0; j < i/2; j++){
    char temp = buffer[j];
    buffer[j] = buffer[i-j-1];
    buffer[i-j-1] = temp;
  }
  buffer[i] = '\0'; // Null-terminate the string
  write_string(buffer);
  return EOF;
}

//--------------------------------------------------------


struct node{
  int value;
  struct node *next;
};

struct node* add_to_list(struct node *head, int i){
  struct node *new_node = simple_malloc(sizeof(struct node));
  if(new_node == NULL){
    return NULL; // Memory allocation failed
  }
  new_node->value = i;
  new_node->next = head;
  return new_node;
}


struct node* delete_node(struct node *head){
  struct node *p = head;
  if(head == NULL){
    return NULL; // List is empty
  }
  head = head->next;
  simple_free(p);
  return head;
}


void free_list(struct node *head){
  struct node *p = head;
  while(p != NULL){
    p = delete_node(p);
  }
}

struct node* reverse_list(struct node* head) {
    struct node *prev = NULL;
    struct node *current = head;
    struct node *next = NULL;

    while (current != NULL) {
        next = current->next; // Store next node
        current->next = prev; // Reverse current node's pointer
        prev = current;       // Move pointers one position ahead
        current = next;
    }
    return prev; // New head of the reversed list
}

void display_list(struct node *head){
  struct node *p = head;
  p = reverse_list(p);
  while (p != NULL){
    write_int(p->value);
    if(p->next != NULL)
      write_string(",");
    p = p->next;
  }
  write_string(";");
  if(p==NULL){
    return;   
  }
}

int main(int argc, char ** argv) {
  struct node *collection = NULL;
  int count = 0;
  char c;
  do 
    {
      c = read_char();
      if (c < 0) {
          break; 
      }
      if(c=='a' || c=='b' || c=='c'){
        if(c=='a')
          collection = add_to_list(collection, count);
        if(c=='c' && collection != NULL)
          collection = delete_node(collection);
        count++;
      }      
    }
  while ((c != 'q' && (c=='a' || c=='b' || c=='c')) ); 
  display_list(collection);
  free_list(collection);
  write_char('\n'); 

  return 0;
}




