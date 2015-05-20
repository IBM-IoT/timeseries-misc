/***************************************************************************
 *
 * Licensed Materials - Property of IBM
 * 
 * Restricted Materials of IBM 
 * 
 * IBM Informix Dynamic Server
 * (c) Copyright IBM Corporation 2013 All rights reserverd.
 *
 *
 *  Title            : json.c
 *  Author           : Cosmo@uk.ibm.com
 *  Description      : JSON functions
 *
 *  Version          : 1.0	2014-03-06 For multi-threaded loader
 *
 ****************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>

/*
** The JSON feature in the TSLoader is not the same as the NoSQL
** server implemntation of JSON/BSON types. As of 12.10.xC5 that
** works with no client side processing required to store data as
** a BSON within TimeSeries.
** 
** This code is to do mapping of a JSON type on the incoming
** message stream and converts it to a normal structured
** TimeSeries type.
*/


/*
** This is a VERY basic JSON document parser. It is built for speed and
** efficiency, not correctness or fancy error handling
*/

#define EAT_COND(_a,_b) {for ( ; *(_a) && (_b); (_a)++)if (*((_a)+1) == '\0') break;}
#define EAT_WHITE(_p)   EAT_COND(_p,(isspace (*_p)))
#define EAT_VALUE(_p)   EAT_COND(_p,(!(isspace (*_p)||*(_p) == ','||*(_p) == '}')))
#define MOVE_TO(_p,_c)  EAT_COND(_p, (*(_p) != (_c)))


/* WARNING: This function modifies the json document.
   A copy should be passed in */

int json_parse_string_pairs (char *json, char ***key_strings, char ***value_strings)
{
  int   nkeys, npairs, nquote;
  char *cptr;

  /* Find the number of { "key" : value } pairs */
  for (nquote = npairs = 0, cptr = json; *cptr; cptr++)
  {
    if (*cptr == '"')
      nquote++;
    else if (*cptr == ':' && ((nquote % 2) == 0))
      npairs++;
  }

  /* Without returning strings arrays only pair count is valid */ 
  if (key_strings == NULL || value_strings == NULL)
    return npairs;

  *key_strings = (char **) calloc (npairs, sizeof (char *));
  *value_strings = (char **) calloc (npairs, sizeof (char *));
  
  for (cptr = json, nkeys = 0; *cptr; cptr++)
  {
    char *eov;

    if (nkeys == 0)
    {
      MOVE_TO (cptr, '{');
      cptr++;
    }

    EAT_WHITE (cptr);

    /* Next thing must be a key */
    if (*cptr != '"')
      return -1;

    /* Point to start of key string */
    (*key_strings)[nkeys] = ++cptr;
    MOVE_TO (cptr, '"');
    *cptr++ = '\0';

    EAT_WHITE (cptr);
    
    /* Next thing must be a separator */
    if (*cptr != ':')
      return -1;

    cptr++;
    EAT_WHITE (cptr);
    
    (*value_strings)[nkeys] = cptr;
    if (*cptr == '"')
    {
      /* value is a string */
      cptr++;
      MOVE_TO (cptr, '\"');
      cptr++;
    }
    else
    {
      EAT_VALUE (cptr);
    }
    nkeys++;
    eov = cptr;
    if (*cptr == '}')
    {
      *eov = '\0';
      break;
    }
    MOVE_TO (cptr, ',');
    *eov = '\0';
  }

  return nkeys;
}

int json_keyncmp (char *k1, char *k2, int len)
{

    for ( ; *k1 && *k2 && len > 0; k1++, k2++, len--)
    {
	if (*k1 == '"')
	    k1++;
    
	if (*k2 == '"')
	    k2++;

	if (! ((*k1 == '_' && *k2 == '.') || (*k1 == '.' && *k2 == '_') ||
	       (tolower(*k1) == tolower(*k2))))
	    return 1;
    }
    return 0;
}

/* search a JSON document for a particular key/value */
int json_lookup (char *lbrace, int len, char *key, char *value)
{
    int   vlen = 0;
    char *cptr, quotedkey[129];

    *value = '\0';
    
    if (*key != '"')
    {
	sprintf (quotedkey, "\"%s\"", key);
	key = quotedkey;
    }

    /* Find the key name */
    if ((cptr = strstr (lbrace, key)) == NULL)
	return 0;

    /* Move past colon in key : value */
    if ((cptr = strchr (cptr, ':')) == NULL)
	return 0;
    cptr++;
    
    /* Eat white space */
    while (*cptr == ' ' || *cptr == '\t' || *cptr == '\n')
      cptr++;

    if (*cptr == '\"')
    {
      char *eptr, *vptr;
      eptr = value;
      for (vlen = 0, vptr = cptr + 1; *vptr != '"'; vlen++, vptr++, eptr++)
	*eptr = *vptr;
      *eptr = '\0';
    }
    else if ((*cptr  >= '0' && *cptr <= '9')
	     || *cptr == '-' || *cptr == '.')
    {
	char *nptr, *vptr;
	for (vlen = 0, nptr = value, vptr = cptr;
	     ((*vptr >= '0' && *vptr <= '9')
	      || *vptr == '-'  || *vptr == '.');
	     vptr++, nptr++, vlen++)
	{
	    *nptr = *vptr;
	}
	*nptr = '\0';
    }
    return vlen;
}
