/* 
 * set.h  from the Set of Integers example, 
 * Ch. 7, C: A Reference Manual 5/e
 * Samuel P. Harbison III and Guy L. Steele Jr.
 *
 * A set package, suitable for sets of small integers in the range 0 to N-1, 
 * where N is the number of bits in an unsigned int type. 
 * Each integer is represented by a bit position; bit i is 1 if and only if 
 * i is in the set. The low-order bit is bit 0. 
 */
#include <limits.h>   /* defines CHAR_BIT */

/* Type SET is used to represent sets. */
typedef unsigned int SET;

/* SET_BITS: Maximum bits per set. */
#define SET_BITS             (sizeof(SET)*CHAR_BIT)

/* check(i): True if i can be a set element. */
#define check(i)             (((unsigned) (i)) < SET_BITS)

/* emptyset: A set with no elements. */
#define emptyset             ((SET) 0)

/* add(s,i): Add a single integer to a set. */
#define add(set,i)           ((set) | singleset (i))

/* singleset(i): Return a set with one element in it. */
#define singleset(i)         (((SET) 1) << (i))

/* intersect: Return intersection of two sets. */
#define intersect(set1,set2) ((set1) & (set2))

/* union: Return the union of two sets. */
#define union(set1,set2)     ((set1) | (set2))

/* setdiff: Return a set of those elements in set1 or set2,
   but not both. */
#define setdiff(set1,set2)   ((set1) ^ (set2))

/* element: True if i is in set. */
#define element(i,set)       (singleset((i)) & (set))

/* forallelements:  Perform the following statement once for
   every element of the set s, with the variable j set to
   that element. To print all the elements in s, just write
   int j;
   forallelements(j, s)
      printf("%d ", j);
*/ 
#define forallelements(j,s) \
    for ((j)=0; (j)<SET_BITS; ++(j)) if (element((j),(s)))

/* first_set_of_n_elements(n): Produce a set of size n whose
   elements are the integers from 0 through n-1.  This
   exploits the properties of unsigned subtractions. */
#define first_set_of_n_elements(n)  (SET)((1<<(n))-1) 

/* next_set_of_n_elements(s): Given a set of n elements,
   produce a new set of n elements.  If you start with the
   result of first_set_of_n_elements(k), and then at each
   step apply next_set_of_n_elements to the previous result,
   and keep going until a set is obtained containing m as a
   member, you will have obtained a set representing all
   possible ways of choosing k things from m things. */
extern SET next_set_of_n_elements(SET x);  /*(CORRECTED 3/31/02)*/

/* printset(s): Print a set in the form "{1, 2, 3, 4}". */
extern void printset(SET z);

/* cardinality(s): Return the number of elements in s. */
extern int cardinality(SET x);

/* print_k_of_n(k,n): Print all the sets of size k having
   elements less than n.  Try to print as many as will fit
   on each line of the output.  Also print the total number
   of such sets; it should equal n!/(k!(n-k)!) 
   where n! = 1*2*...*n. */
extern void print_k_of_n(int k, int n);
