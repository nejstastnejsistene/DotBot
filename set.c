#include <stdio.h>
#include "set.h"

/* 
 * set.c from the Set of Integers example, 
 * Ch. 7, C: A Reference Manual 5/e
 * Samuel P. Harbison III and Guy L. Steele Jr.
 */


int cardinality(SET x)
{   
/* The following loop body is executed once for every 1-bit
   in the set x.  Each iteration, the smallest remaining
   element is removed and counted. The expression (x & -x)
   is a set containing only the smallest element in x, in
   twos-complement arithmetic. */
    int count = 0;
    while (x != emptyset) {
        x ^= (x & -x);
        ++count;
    }
    return count;
}

SET next_set_of_n_elements(SET x)
{
/* This code exploits many unusual properties of unsigned
   arithmetic.  As an illustration:
     if x               == 001011001111000, then
     smallest           == 000000000001000
     ripple             == 001011010000000
     new_smallest       == 000000010000000
     ones               == 000000000000111
     the returned value == 001011010000111 
   The overall idea is that you find the rightmost
   contiguous group of 1-bits. Of that group, you slide the
   leftmost 1-bit to the left one place, and slide all the
   others back to the extreme right. 
   (This code was adapted from HAKMEM.) */
    SET smallest, ripple, new_smallest, ones;
    if (x == emptyset) return x;
    smallest     = (x & -x); 
    ripple       = x + smallest;
    new_smallest = (ripple & -ripple);
    ones         = ((new_smallest / smallest) >> 1) - 1;
    return (ripple | ones);
} 

void printset(SET z)
{
    int first = 1;
    int e;
    forallelements(e, z) {
        if (first) printf("{");
        else printf(", ");
        printf("%d", e);
        first = 0;
    }
    if (first) printf("{"); /* Take care of emptyset */
    printf("}");            /* Trailing punctuation */
}

#define LINE_WIDTH 54
void print_k_of_n(int k, int n)
{
    int count = 0;
    int printed_set_width = k * ((n > 10) ? 4 : 3) + 3;
    int sets_per_line = LINE_WIDTH / printed_set_width;
    SET z = first_set_of_n_elements(k);
    printf("\nAll the size-%d subsets of ", k);
    printset (first_set_of_n_elements(n));
    printf(":\n");
    do {            /* Enumerate all the sets. */
        printset(z);
        if ((++count) % sets_per_line) printf (" ");
        else printf("\n");
        z = next_set_of_n_elements(z);
    }while ((z != emptyset) && !element(n, z));
    if ((count) % sets_per_line) printf ("\n");
    printf("The total number of such subsets is %d.\n",
        count);
}
