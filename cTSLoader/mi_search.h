/***************************************************************************
 *
 * Licensed Materials - Property of IBM
 * 
 * Restricted Materials of IBM 
 * 
 * IBM Informix Dynamic Server
 * (c) Copyright IBM Corporation 2011 All rights reserverd.
 *
 *
 *  Title            : mi_search.h 
 *  Description      : TimeSeries Cache Loader - IDS safe hash functions
 *
 ****************************************************************************
 */

#include "mitypes.h"

/* These are the Informix equivalents */
#define HSEARCH_R  mi_hsearch_r
#define HCREATE_R  mi_hcreate_r
#define HDESTROY_R mi_hdestroy_r
#define HSTATS     mi_hstats
#define HASH_HDR   struct mi_hsearch_data
#define ACTION     mi_ACTION
#define ENTRY      mi_ENTRY

#define MALLOC     malloc
#define REALLOC	   realloc
#define FREE       free
#define HALLOC_R   hash_alloc
#define HFREE_R

#define HASH_BASE  (10)
#define DEC_NUMERIC   0x100
#define HEX_NUMERIC   0x200
#define NUM_MASK      0x0FF

#ifdef DETECT_COLLISIONS

/* Steal a bit from the stored hash to speed up FIND of missing values */
#define HASH_MASK       0x7FFFFFFF
#define COLLIDE_MASK    0x80000000
#define HASHBITS(x)     ((x) & HASH_MASK)
#define COLLIDED(x)     (((x) & COLLIDE_MASK) == COLLIDE_MASK)
#define COLLISION(x)    ((x) |= COLLIDE_MASK)
#define HASH_EQUAL(a,b) (HASHBITS(a) == HASHBITS(b))
#else
#define HASHBITS(x)     (x)

/* Always assume collision if this feature turned off */
#define COLLIDED(x)     (1)
#define COLLISION(x)
#define HASH_EQUAL(a,b) ((a) == (b))
#endif

/* Don't use the regular hsearch structures.
   We can use optimised versions instead. */

/* Traditionaly item.data is a pointer but we can save a bit of memory on
   64-bit platforms by using a 4 byte int to store an array counter.
   The hash value is kept here as well which reduces the number of calls
   to strcmp() but increases space usage. */
typedef struct mi_ENTRY_s
{
    char *key;
#ifdef USE_OFFSET
    uint4 offset;
#else
    void *data;
#endif
    uint4 hash;
} mi_ENTRY;

typedef struct HASH_BLK_s
{
    struct HASH_BLK_s *next_block;
    void *next_alloc;
    int mem_free;
} HASH_BLK;

typedef enum
{ TSL_SKHASH, TSL_CHHASH }
tsl_alg_enum;

typedef struct mi_hsearch_data
{
    tsl_alg_enum algorithm_type;        /* type of algorithm to use */
    int hash_base;              /* hash base selected by user if applicable */
    int size;                   /* Size of hash table requested */
    int nitems;                 /* Number of items inserted */
    int total_bytes;            /* Total size of current allocation */
    int h_bits;                 /* total number of bits for the other hash algorithm */
    int prime;                  /* Hash Table size is a prime greater than items requested */
    long ncollide;               /* Can be used to optimise FIND on a perfect hash */
    uint2 *numarray;            /* Array to optimise hashing function */

#ifdef COLLECT_STATS
    /* Only collect these stats when debugging */
    int reinsert;
    int imax;
#endif
    mi_ENTRY *table;
    HASH_BLK *mem_block;
} mi_HASH_HDR;

typedef enum
{ FIND, ENTER }
mi_ACTION;

int mi_hcreate_r (
    size_t n,
    mi_HASH_HDR * htab);

void mi_hdestroy_r (
    mi_HASH_HDR * htab);

void mi_hstats (
    mi_HASH_HDR * htab);

int mi_hsearch_r (
    mi_ENTRY item,
    mi_ACTION action,
    mi_ENTRY ** retval,
    mi_HASH_HDR * htab);

void *hash_alloc (
    int len,
    mi_HASH_HDR * h);
