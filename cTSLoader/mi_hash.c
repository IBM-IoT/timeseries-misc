/***************************************************************************
 *
 * Licensed Materials - Property of IBM
 * 
 * Restricted Materials of IBM 
 * 
 * IBM Informix Dynamic Server
 * (c) Copyright IBM Corporation 2011,2013
 *
 *
 *  Title            : mi_hash.c 
 *  Description      : TimeSeries Cache Loader - IDS safe hash functions
 *
 ****************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "mi.h"
#include "milib.h"

/* #define COLLECT_STATS     1 */
/* #define DETECT_COLLISIONS 1 */

#include "mi_search.h"

/* this section taken from /vobs/tirstarm/miapi/miapi/sapimem.c */

/*------------------------------------------------------------------------
**
** Hash functions
**
** Taken from:
**	Jenkins, Robert J., Jr, Hash Functions for Hash Table Lookup, 1997
**	http://ourworld.compuserve.com/homepages/bob_jenkins/evahash.htm
**
*/

/*
--------------------------------------------------------------------
lookup2.c, by Bob Jenkins, December 1996
hash(), and MIX() are externally useful functions.
You can use this free for any purpose.  It has no warranty.
--------------------------------------------------------------------
*/

#define HASHSIZE(n) ((muint)1<<(n))
#define HASHMASK(n) (HASHSIZE(n)-1)

/*
--------------------------------------------------------------------
MIX -- mix 3 32-bit values reversibly.
For every delta with one or two bit set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If MIX() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If MIX() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
MIX() takes 36 machine instructions, but only 18 cycles on a superscalar
  machine (like a Pentium or a Sparc).  No faster mixer seems to work,
  that's the result of my brute-force search.  There were about 2^^68
  hashes to choose from.  I only tested about a billion of those.
--------------------------------------------------------------------
*/
#define MIX(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8);  \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12); \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5);  \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}


/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k     : the key (the unaligned variable-length array of bytes)
  len   : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 36+6len instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & HASHMASK(10));
In which case, the hash table should have HASHSIZE(10) elements.

If you are hashing n strings (unsigned char **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  74512.261@compuserve.com.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://ourworld.compuserve.com/homepages/bob_jenkins/evahash.htm
Use for hash table lookup, or anything where one collision in 2^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

static muint
hash (
    register unsigned char *k,  /* the key */
    muint length,               /* the length of the key */
    muint initval)
{                               /* the previous hash, or an arbitrary value */
    register muint a,
      b,
      c,
      len;

    /* Set up the internal state */
    len = length;
    a = b = 0x9e3779b9;         /* the golden ratio; an arbitrary value */
    c = initval;                /* the previous hash value */

   /*---------------------------------------- handle most of the key */
    while (len >= 12) {
        a += (k[0] + ((muint) k[1] << 8) + ((muint) k[2] << 16) +
              ((muint) k[3] << 24));
        b += (k[4] + ((muint) k[5] << 8) + ((muint) k[6] << 16) +
              ((muint) k[7] << 24));
        c += (k[8] + ((muint) k[9] << 8) + ((muint) k[10] << 16) +
              ((muint) k[11] << 24));
        MIX (a, b, c);
        k += 12;
        len -= 12;
    }

   /*------------------------------------- handle the last 11 bytes */
    c += length;
    switch (len) {              /* all the case statements fall through */
    case 11:
        c += ((muint) k[10] << 24);
    case 10:
        c += ((muint) k[9] << 16);
    case 9:
        c += ((muint) k[8] << 8);
        /* the first byte of c is reserved for the length */
    case 8:
        b += ((muint) k[7] << 24);
    case 7:
        b += ((muint) k[6] << 16);
    case 6:
        b += ((muint) k[5] << 8);
    case 5:
        b += k[4];
    case 4:
        a += ((muint) k[3] << 24);
    case 3:
        a += ((muint) k[2] << 16);
    case 2:
        a += ((muint) k[1] << 8);
    case 1:
        a += k[0];
        /* case 0: nothing left to add */
    }
    MIX (a, b, c);

   /*-------------------------------------------- report the result */
    return c;
}

/* The standard C version using hsearch_r will break in a multi-CPU VP
   environment as the internally memory is not created with mi_alloc.
   So we must re-write some functional equivalents */

/* We assume that the key "string" is in fact mostly numeric and this
   can be leveraged to efficiently turn the string into a number for
   hashing. The number string should be fairly long for this to work. */

#define HASH_BLK_SIZE   (1024 * 16)
#define HASH_MAX_BLKS   1024
#define HASH_CHECK_MEM(h,len) ((h && h->mem_block == NULL) || (h && h->mem_block && h->mem_block->mem_free < len))

void *
hash_alloc (
    int len,
    mi_HASH_HDR * h)
{
    void *ptr = NULL;

    /* align the len to mi_pointer boundry
     */
    len = MI_ALIGNBYTES (len, sizeof (mi_pointer));

    if (HASH_CHECK_MEM (h, len)) {
        HASH_BLK *new_block = NULL;
        int nblocks = (((size_t) h->size / 8) * len) / HASH_BLK_SIZE;

        if (nblocks <= 0)
            nblocks = 1;
        if (nblocks > HASH_MAX_BLKS)
            nblocks = HASH_MAX_BLKS;

        if ((new_block =
             (HASH_BLK *) MALLOC (HASH_BLK_SIZE * nblocks)) == NULL)
            return NULL;

        new_block->next_block = NULL;
        new_block->next_alloc = (void *) (new_block + 1);
        new_block->mem_free = (HASH_BLK_SIZE * nblocks) - sizeof (HASH_BLK);

        new_block->next_block = (h->mem_block ? h->mem_block : NULL);
        h->mem_block = new_block;
    }

    ptr = h->mem_block->next_alloc;
    h->mem_block->next_alloc = (char *) h->mem_block->next_alloc + len;
    h->mem_block->mem_free -= len;

    return (ptr);
}

/* Find the next prime number */
static uint4
nextprime (
    uint4 n)
{
    uint4 d;

    for (n |= 1; n > 0; n += 2) {
        for (d = 3; (d * d) < n && (n % d) != 0; d += 2);
        if ((n % d) != 0)
            break;
    }
    return n;
}

/* Sequential Key Hash
   This hash function is highly optimised for strings that are mostly ints.
   For numeric sequential keys it is a Perfect Hash - which is nice. */
static uint4
tsl_skhash (
    char *key,
    int len,
    int base,
    uint2 * numarray)
{
    char *kptr;
    uint4 h = 0,
        i = 0;
    uint2 x;
    int hbase = base < 16 ? 16 : base;

    for (h = 0, i = 0, kptr = key; *kptr; kptr++) {
        /* We hold the kptr->numeric mapping in an array for speed */
        x = numarray[(int)(*kptr)];
        if (x & DEC_NUMERIC)
            i = (i * base) + (x & NUM_MASK);
        else if (x & HEX_NUMERIC)
            i = (i * hbase) + (x & NUM_MASK);
        else {
            h += i + x;
            i = 0;
        }
    }
    h += i;

    return h;
}

/* The number of items passed in should include a degree of over estimation
   to make the hash more effective. The size is rounded up to the next
   prime number */
int
mi_hcreate_r (
    size_t n,
    mi_HASH_HDR * htab)
{
    int b;
    int nbits = 0;
    size_t bytes = 0;

    if (htab == NULL)
        return 0;

    htab->prime = nextprime (n);
    htab->nitems = 0;
    htab->size = n;
    htab->h_bits = 0;
    htab->total_bytes = 0;
    htab->mem_block = NULL;

    switch (htab->algorithm_type) {
    case TSL_SKHASH:
        /* Build the array for calculating hash values */
        htab->numarray = (uint2 *) MALLOC (0x100 * sizeof (uint2));
        if (htab->numarray) {
            for (b = 0; b < 0x100; b++) {
                switch (b) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    /* Set the numeric bit */
                    htab->numarray[b] = (b - '0') | DEC_NUMERIC;
                    break;
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                    /* Set the numeric & hex bits */
                    htab->numarray[b] = ((b - 'A') + 10) | HEX_NUMERIC;
                    break;
                    /* Set the numeric & hex bits */
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    htab->numarray[b] = ((b - 'a') + 10) | HEX_NUMERIC;
                    break;
                default:
                    htab->numarray[b] = (b);
                }
            }
        }
        bytes = htab->prime * sizeof (mi_ENTRY);
        break;
    case TSL_CHHASH:
        for (nbits = 2; HASHSIZE (nbits) < n; nbits++); /*nada */
        htab->h_bits = nbits;
        bytes = HASHSIZE (nbits) * sizeof (mi_ENTRY);
        break;
    }
    htab->table = (mi_ENTRY *) MALLOC (bytes);
    htab->total_bytes = bytes;

    if (htab->table)
        memset (htab->table, 0, bytes);

#ifdef COLLECT_STATS
    htab->reinsert = htab->imax = htab->ncollide = 0;

    printf ("Allocating %d items in table for %d entries %ld bytes using %s\n",
            htab->prime, htab->size, bytes,
	    htab->algorithm_type == TSL_SKHASH ? "TSL_SKHASH" : (
		htab->algorithm_type == TSL_CHHASH ? "TSL_CHHASH" : "????"));
#endif

    return htab->table == NULL ? 0 : 1;
}


void
mi_hstats (
    mi_HASH_HDR * htab)
{
#ifdef COLLECT_STATS
    long n = 0;
    int  i;
    
#ifdef DETECT_COLLISIONS
    for (n = i = 0; i < htab->prime; i++) {
        mi_ENTRY *f = &htab->table[i];
        if (COLLIDED (f->hash))
            n++;
        /* printf ("[%d] = '%s' 0x%x\n", i, f->key, f->hash); */
    }
#endif

    printf
        ("Allocated %d items in table, inserted %d entries - %ld collisions, %d re-insertions, maxinc %d, collided keys %ld\n",
         htab->prime, htab->nitems, htab->ncollide, htab->reinsert,
         htab->imax, n);
#endif

}



void
mi_hdestroy_r (
    mi_HASH_HDR * htab)
{
    if (htab == NULL)
        return;

#ifdef COLLECT_STATS
    mi_hstats (htab);
#endif
    
    if (htab->table)
        FREE (htab->table);
    if (htab->numarray)
        FREE (htab->numarray);

    htab->table = NULL;
    htab->numarray = NULL;

    /* Free the block allocator list */
    while (htab->mem_block) {
        HASH_BLK *hb = htab->mem_block->next_block;
        FREE (htab->mem_block);
        htab->mem_block = hb;
    }

    return;
}

int
mi_hsearch_r (
    mi_ENTRY item,
    mi_ACTION action,
    mi_ENTRY ** retval,
    mi_HASH_HDR * htab)
{
    int keylen;
    mi_ENTRY *found;
    uint4 h32 = 0;
    mi_boolean outofspacecond = MI_FALSE;
    int origin,
      offset = 0,
      inc;

    keylen = strlen (item.key);
    
    if (htab == NULL || htab->table == NULL)
        return MI_ERROR;

    switch (htab->algorithm_type) {
    case TSL_SKHASH:
        h32 =
            HASHBITS (tsl_skhash
                      (item.key, keylen, htab->hash_base, htab->numarray));
        offset = h32 % htab->prime;
        outofspacecond =
            ((htab->nitems * 1.25) >= htab->size ? MI_TRUE : MI_FALSE);
        break;
    case TSL_CHHASH:
        h32 = hash ((unsigned char *) item.key, (muint) keylen, 0);
        offset = h32 & HASHMASK (htab->h_bits);
        outofspacecond =
            (((htab->nitems * 1.25) >=
              HASHSIZE (htab->h_bits)) ? MI_TRUE : MI_FALSE);
        break;
    }
    found = &htab->table[offset];
    if (action == ENTER && outofspacecond)
        return -3;

    if (found->key) {
        if (strcmp (found->key, item.key) == 0) {
            /* Got it first go! */
            goto found;
        }
        else {
            /* If there were no collisions for this hash then we can stop now.
               The global collision check is only relevant when a Perfect
               Hash has been produced, which is only possible with integer
               sequential key values. */
            if (action == FIND
                && (htab->ncollide == 0 || !COLLIDED (found->hash)))
                goto not_found;

            /* There was a hash collision, so we must find a new slot */

            /* Use Knuth's double hash */
            for (origin = offset, inc = 0;;) {
                /* Only count collisions when building the table. */
                if (action == ENTER) {
                    htab->ncollide++;
                    COLLISION (found->hash);
                }

#ifdef COLLECT_STATS
                if (++inc > htab->imax)
                    htab->imax = inc;
#endif

                offset -= 2;
                if (offset < 0)
                    offset += htab->prime;

                found = &htab->table[offset];
                if (found->key == NULL)
                    goto not_found;

                /* Back to original position? */
                if (offset == origin)
                    goto not_found;

                /* We can check the original hash value before doing an
                   expensive strcmp  */
                if ((HASH_EQUAL (found->hash, h32))
                    && (strcmp (found->key, item.key) == 0))
                    goto found;

                /* Bail if we've seen the whole table - This shouldn't happen! */
                if (inc >= htab->prime)
                    return -2;
            }
        }
    }

  not_found:

    if (found->key == NULL) {
        if (action == ENTER) {
            found->key = item.key;
#ifdef USE_OFFSET
            found->offset = item.offset;
#else
	    found->data = item.data;
#endif
            found->hash = h32;
            *retval = found;
            htab->nitems++;
            return 1;
        }
    }

    return 0;


  found:

    if (found->key) {
        *retval = found;
        if (action == ENTER) {
            /* We found the same item again? - we overwrite the key and data
               values because that's what the system one does.
               But it may be more useful to return the slot pointer and let
               the calling routine sort it out. */
            found->key = item.key;
#ifdef USE_OFFSET
            found->offset = item.offset;
#else
            found->data = item.data;
#endif
            found->hash = h32;

#ifdef COLLECT_STATS
            htab->reinsert++;
#endif
        }
        return 1;
    }

    return 0;
}
