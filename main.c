#include "io.h"
#include "mm.h"

char msg[] = "\n ** You need to copy your main.c file from Assignment 1 **\n";

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




