#include <stdlib.h>
#include <stdio.h>

int main (int argc, char **argv){

  char* x = malloc(24);
  char* y = malloc(19);
  char* z = malloc(32);

  // below is my test code for part 4
  *x = 'a';
  *(x+1) = 'b';
  
  printf("Test code for part4:\nx = %p\n", x);
  printf("y = %p\n", y);
  printf("z = %p\n", z);

  x = realloc(x, 26);
  printf("x address for bigger realloc = %p\nx[0] = %c\nx[1] = %c\n", x, *x, *(x+1));
  x = realloc(x, 10);
  printf("x address for smaller realloc = %p\nx[0] = %c\nx[1] = %c (should be the same as above)\n\n", x, *x, *(x+1)); 

  free(x);
  free(y);
  free(z);

  // below is my test code for part 5
  char* a = malloc(24);
  char* b = malloc(19);
  char* c = malloc(32);

  *a = 'h';
  *(a+1) = 'i';

  *c = 'y';
  *(c+1) = 'o';

  printf("Test code for part5:\na = %p\n", a);
  printf("b = %p\n", b);
  printf("c = %p\n\n", c);

  // deallocate b
  free(b); // free the block
  b = NULL; // set pointer to null

  printf("After deallocating b:\n");
  printf("a = %p\n", a);
  printf("FREED b = %p\n", b); // expecting NULL
  printf("c = %p\n\n", c);


  // realloc of c with bigger a
  a = realloc(a, 40);
  printf("Bigger realloc of a:\n");
  printf("BIGGER REALLOC of a = %p\nValue of a[0] (should be h): %c\nValue of a[1] (should be i): %c\n", a, *a, *(a+1));
  printf("c = %p\n(a should have different address)\n\n", c);


  // realloc of c with smaller c
  c = realloc(c, 16);
  printf("Smaller realloc of c:\n");
  printf("a = %p\n", a);
  printf("SMALLER REALLOC of c = %p\n(c should have same address)\nValue of c[0] (should be y): %c\nValue of c[1] (should be o): %c\n\n", c, *c, *(c+1));

  // allocate more memory
  char *d = malloc(28);
  printf("After allocating new char d:\n");
  printf("a = %p\n", a);
  printf("c = %p\n", c);
  printf("d = %p\n\n", d);

  // free all allocated memory blocks
  free(a);
  free(c);
  free(d);

  // set pointers to NULL
  a = NULL;
  c = NULL;
  d = NULL;

  printf("AFTER freeing all memory:\n");
  printf("a (after free should be NULL): %p\n", a); // should be null after deallocation
 printf("c (after free should be NULL): %p\n", c); // should be null after deallocation
 printf("d (after free should be NULL): %p\n", d); // should be null after deallocation

  // below is my test code for part 6 (uncomment debug statements) 
  char* e = malloc(10);
  char* f = malloc(2);
  char* g = malloc(15);

  free(g);
  free(f);
  free(e);
}
