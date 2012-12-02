/*****************************************************************************

	 IMPORTANT: You *must* use palloc0 and pfree, not malloc and free, in your
	 implementation.  This will allow your filter to integrate into PostgreSQL.

******************************************************************************/

#include "postgres.h"
#include "utils/cmsketch.h"

/* initialize the count-min sketch for the specified width and depth */
cmsketch* init_sketch(uint32 width, uint32 depth) {
  cmsketch *sketch = (cmsketch *) palloc0(sizeof(cmsketch));
  sketch->cmdata = palloc0((width * depth) * sizeof(uint32));
  if (sketch->cmdata == NULL) {
    fprintf(stderr, "out of memory\n");
    return NULL;
  }
  sketch->cmdataWidth = width;
  sketch->cmdataDetph = depth;
  return sketch;
}

/* increment 'bits' in each sketch by 1. 
 * 'bits' is an array of indices into each row of the sketch.
 *    Thus, each index is between 0 and 'width', and there are 'depth' of them.
 */
void increment_bits(cmsketch* sketch, uint32 *bits) {
  uint32 row;
  uint32 column;
  uint32 width = sketch->cmdataWidth;
  uint32 depth = sketch->cmdataDepth;
  for (row = 0; row < depth; row++) {
    column = bits[row];
    sketch->cmdata[width * row + column] += 1;
  }
}

/* decrement 'bits' in each sketch by 1.
 * 'bits' is an array of indices into each row of the sketch.
 *    Thus, each index is between 0 and 'width', and there are 'depth' of them.
 */
void decrement_bits(cmsketch* sketch, uint32 *bits) {
  uint32 row;
  uint32 column;
  uint32 width = sketch->cmdataWidth;
  uint32 depth = sketch->cmdataDepth;
  for (row = 0; row < depth; row++) {
    column = bits[row];
    if (sketch->cmdata[width * row + column] != 0) { 
      sketch->cmdata[width * row + column] -= 1;
    }
  }
}

/* return the minimum among the indicies pointed to by 'bits'
 * 'bits' is an array of indices into each row of the sketch.
 *    Thus, each index is between 0 and 'width', and there are 'depth' of them.
 */
uint32 estimate(cmsketch* sketch, uint32 *bits) {
  uint32 row;
  uint32 column;
  uint32 width = sketch->cmdataWidth;
  uint32 depth = sketch->cmdataDepth;
  uint32 minimum = UINT_MAX;
  uint32 current;
  for (row = 0; row < depth; row++) {
    column = bits[row];
    current = sketch->cmdata[width * row + column];
    if (current < minimum) {
      minimum = current;
    }
  }
  return minimum;
}

/* set all values in the sketch to zero */
void reset_sketch(cmsketch* sketch) {
  uint32 sizeOfSketch = sketch->cmdataWidth * sketch->cmdataDepth;
  uint32 i;
  for (i = 0; i < sizeOfSketch; i++) {
    sketch->cmdata[i] = 0;
  }
}

/* destroy the sketch, freeing any memory it might be using */
void destroy_sketch(cmsketch* sketch) {
  pfree(sketch);
}
