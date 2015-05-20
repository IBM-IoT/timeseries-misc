/***************************************************************************
 *
 * Licensed Materials - Property of IBM
 * 
 * Restricted Materials of IBM 
 * 
 * IBM Informix Dynamic Server
 * (c) Copyright IBM Corporation 2014 All rights reserverd.
 *
 *
 *  Title            : hash.c
 *  Author           : Cosmo@uk.ibm.com
 *  Description      : TimeSeries Loader - hash functions
 *
 *  Version          : 1.1	2014-03-06 Wrappers for system hash functions
 *
 ****************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "mi_search.h"

#include "defs.h"

int hash_create (HASH_HDR *hash_table, size_t n_entries)
{
  bzero (hash_table, sizeof (HASH_HDR));

  hash_table->algorithm_type = TSL_CHHASH;
  
  /* Increase the actual size by 25% to make the hash more efficient */
  return HCREATE_R (n_entries * 1.25, hash_table); 
}

void hash_destroy (HASH_HDR *hash_table)
{
  HDESTROY_R (hash_table); 
}

void *hash_insert (HASH_HDR *hash_table, char *key, void *data)
{
  ENTRY e, *new;

  e.key = key;
  e.data = data;

  if (HSEARCH_R (e, ENTER, &new, hash_table) == 0 || new == NULL)
  {
      //printf ("hash_insert: FAILED for key <%s>\n", e.key);
      return NULL;
  }

  return e.data;
}

void *hash_search (HASH_HDR *hash_table, char *key)
{
    ENTRY e, *r;

    e.key = key;

    if (HSEARCH_R (e, FIND, &r, hash_table) == 0 || r == NULL)
    {
	//printf ("hash_search: FAILED for <%s>\n", e.key);
	return NULL;
    }
    
    return (void *) r->data;
}
