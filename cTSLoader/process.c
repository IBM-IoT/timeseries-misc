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
 *  Title            : process.c
 *  Author           : Cosmo@uk.ibm.com
 *  Description      : Utility routines for Parallel TimeSeries Loader
 *
 *  Version          : 1.0	2014-03-06 Created for multi-threaded loader 
 *                     2.0	2015-04-01 Split out utility functions       
 *
  ****************************************************************************
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <libgen.h>
#include <pthread.h>
#include <signal.h>

#include "mi_search.h"

#include "defs.h"

int  end_process = 0;

db_thread *find_thread (int type)
{
    int i;
    
    for (i = 0; i < thread_total; i++)
	if (tlist[i]->type == type)
	    return tlist[i];
    
    return NULL;
}

int find_server (db_thread *t, int nodeid)
{
    int       i;
    global   *g;
    node     *n;

    g = t->globals;

    for (i = 0; i < g->nservers; i++)
    {
	n = &(g->server_list[i]);
	if (n->id == nodeid)
	    return i;
    }

    printf ("Error: Couldn't find server for node %d\n", nodeid);
    
    return -1;
}

int get_message (db_thread *t, hdr_t *hdr, char *line)
{
    int     inlen = 0;
    size_t  bytes;
    global *g;

    g = t->globals;
    
    if (g->verbose > 1)
	printf ("%s: Reading from fd %d\n", t->name, t->pipe);

    /* Told to exit */
    if (t->treturn != 0 || end_process)
    {
	inlen = CMD_HDR;
	hdr->dev_id = CMD_TERMINATE;
	goto end;
    }
    if ((bytes = read (t->pipe, hdr, sizeof (hdr_t))) != sizeof (hdr_t))
    {
	/* Told to exit */
	if (t->treturn != 0 || end_process)
	{
	    inlen = CMD_HDR;
	    hdr->dev_id = CMD_TERMINATE;
	}
	else
	{
	    printf ("%s: Could not read next message header - %d/%d bytes\n",
		t->name, (int)bytes, (int)sizeof (hdr_t));
	    inlen = -1;
	}
	goto end;
    }
    inlen = hdr->len;
    if (inlen == CMD_HDR)
    {
	if (g->verbose > 1)
	    printf ("%s: Command %d received\n", t->name, hdr->dev_id);
    }
    else if ((bytes = read (t->pipe, line, inlen)) != inlen)
    {
	printf ("%s: Only got %d of %d bytes of next message\n",
		t->name, (int)bytes, inlen);
	line[inlen] = '\0';
    }

end:
    
    return inlen;
}

char *find_field (char *in, int field, char ifs)
{
    char *inptr = in;
    int   f = 0;
    
    for (f = 0, inptr = in; *inptr; )
    {
	if (*inptr++ == ifs)
	{
	    f++;
	    if (f == field)
		return inptr;
	}
    }
    return NULL;    
}

int fieldcmp (char *aptr, char *bptr, char ifs)
{
    for ( ; *aptr && *bptr && *aptr != ifs && *bptr != ifs; aptr++, bptr++)
	if (*aptr != *bptr)
	    break;
    
    if (*aptr != *bptr)
	return 1;

    return 0;
}

int fieldcpy (char *to, char *from, char ifs)
{
    int count;

    for (count = 0; *from && *from != ifs; count++)
	*to++ = *from++;

    *to = '\0';
    
    return count;
}

int remove_repeats (char *line, int inlen, ts_data *d, char ifs, 
		    int start_col, int skip_cols, int remove_all_nulls,
		    int repeat_marker)
{
    char outbuf[LINE_SIZE], prevbuf[LINE_SIZE],
	*prevptr, *outptr, *fields, *lptr, *inptr;
    int  fields_len, new, outlen, col;

    fields = find_field (line, start_col, ifs);

    if (fields == NULL)
	return inlen;

    fields_len = fields - line;

    if (d->last_out == NULL)
    {
	d->last_out_len = inlen - fields_len;
	d->last_buf_len = d->last_out_len + 1;
	d->last_out = (char *) calloc (d->last_buf_len, 1);
	memcpy (d->last_out, fields, d->last_out_len);
	d->last_out[d->last_out_len] = '\0';
	d->repeat_count = 0;
	return inlen;
    }
    
    *(outptr = outbuf)   = '\0';
    *(prevptr = prevbuf) = '\0';

    for (col = new = 0, inptr = fields, lptr = d->last_out; inptr && lptr ; col++)
    {
	int ferr = fieldcmp (inptr, lptr, ifs);

	if (*inptr != ifs && ferr == 1)
	{
	    /* New field value - copy to output and previous */
	    outptr += fieldcpy (outptr, inptr, ifs); 
	    prevptr += fieldcpy (prevptr, inptr, ifs);
	    new++;
	}
	else if (*lptr != ifs)
	{
	    /* Always copy the skipped fields */
	    if (col < skip_cols)
	    {
		if (*inptr == ifs)
		    outptr += fieldcpy (outptr, lptr, ifs);
		else
		    outptr += fieldcpy (outptr, inptr, ifs);
	    }
	    prevptr += fieldcpy (prevptr, lptr, ifs);
	}
	lptr = find_field (lptr, 1, ifs);
	inptr = find_field (inptr, 1, ifs);

	*prevptr++ = '|'; *prevptr = '\0';
	*outptr++ = '|';  *outptr = '\0';

	if (inptr == NULL || *inptr == '\0')
	    break;	
    }
    
    if (new)
    {
	d->last_out_len = (prevptr - prevbuf);
	/* Expand previous buffer if required */
	if (d->last_buf_len < (d->last_out_len + 1))
	{
	    free (d->last_out);
	    d->last_buf_len = d->last_out_len + 1;
	    d->last_out = (char *) calloc (d->last_buf_len, 1);
	}
	/* This is a merge of previous + new */
	memcpy (d->last_out, prevbuf, d->last_out_len);
	d->last_out[d->last_out_len] = '\0';
    }
    
    /* Sync up to known non-NULL fields at marker interval */
    if (++d->repeat_count >= repeat_marker)
    {
	outlen = d->last_out_len;
	outptr = d->last_out;
	d->repeat_count = 0;
    }
    else
    {
	/* Use compacted output buffer */
	outlen = outptr - outbuf;
	outptr = outbuf;
    }
    memcpy (fields, outptr, outlen);
    fields[outlen] = '\0';

    if (new == 0 && remove_all_nulls)
	return 0;
    
    return (fields_len + outlen); 
}

int log_reject (char *reject_file, char *reject_line)
{
    int rv = 0;    
    FILE *rfile;
    
    if (reject_file)
    {
	/* This should have already been checked in main */
	if ((rfile = fopen (reject_file, "a")) == NULL)
	    rv = -1;
	else
	{
	    fprintf (rfile, "%s\n", reject_line);	    
	    fclose (rfile);
	}	
    }
    else
	rv = 1;

    return rv;
}

time_t parse_line (char *in, int len, int tstamp_col, char ifs,
		   time_t *last_local_time, char **ts_start,
		   int *p_year, int *p_month, int *p_day)
{
  int        nfields, year, month, day, hour, minute, second;
  char      *cptr, *cend, sep1 = ifs, sep2 = ifs, *sep1ptr;
  struct tm *utc, local;
  time_t     local_time;

  cend = in + len;
  for (nfields = 1, cptr = in; cptr < cend; cptr++)
  {
      if (*cptr == ifs)
	  *cptr = '|';
      if (*cptr == '|')
	  nfields++;
      if (nfields == tstamp_col)
      {
	  if (cptr[1] != '|')
	  {
	      if (sscanf (cptr + 1, "%d-%d-%d", &year, &month, &day) != 3)
	      {
		  printf ("ERROR parsing date at <%s>\n", cptr);
		  exit (1);
	      }
	      *p_year = year;
	      *p_month = month;
		*p_day = day;
		sep1ptr = cptr + 11;
		sep1 = *sep1ptr;
	    }
	  else
	  {
	      year = *p_year;
	      month = *p_month;
	      day = *p_day;
	      sep1ptr = cptr + 1;
	    }
	  if (sscanf (sep1ptr + 1, "%d:%d:%d%c", &hour, &minute, &second, &sep2) != 4)
	  {
	      printf ("ERROR parsing time at <%s>\n", sep1ptr + 1);
	      exit (1);
	  }
	  local.tm_sec = second;
	  local.tm_min = minute;
	  local.tm_hour = hour;
	  local.tm_mday = day;
	  local.tm_mon = month - 1;
	  local.tm_year = year - 1900;
	  local.tm_isdst = -1;
	  local_time = mktime (&local);
	  
	  if (last_local_time)
	  {
	      /* Some hackery to remove the duplicates during the 2am change.
	     It relies on the data being pre-ordered on timestamp so this:
	     
	     key1|2011-10-30 01:45:00.00000|08|
	     key1|2011-10-30 02:00:00.00000|09|
	     key1|2011-10-30 02:15:00.00000|10|
	     key1|2011-10-30 02:30:00.00000|11|
	     key1|2011-10-30 02:45:00.00000|12|
	     key1|2011-10-30 02:00:00.00000|13|
	     key1|2011-10-30 02:15:00.00000|14|
	     key1|2011-10-30 02:30:00.00000|15|
	     key1|2011-10-30 02:45:00.00000|16|
	     key1|2011-10-30 03:00:00.00000|17|
	     
	     Must be presented as:
	     
	     key1|2011-10-30 01:45:00.00000|08|
	     key1|2011-10-30 02:00:00.00000|09|
	     key1|2011-10-30 02:00:00.00000|13|
	     key1|2011-10-30 02:15:00.00000|10|
	     key1|2011-10-30 02:15:00.00000|14|
	     key1|2011-10-30 02:30:00.00000|11|
	     key1|2011-10-30 02:30:00.00000|15|
	     key1|2011-10-30 02:45:00.00000|12|
	     key1|2011-10-30 02:45:00.00000|16|
	     key1|2011-10-30 03:00:00.00000|17|
	     
	      */
	  
	      if (local_time == *last_local_time && local.tm_isdst == 1)
	      {
		  local.tm_isdst = 0;
		  local_time += 3600;
	      }
	      else
		  *last_local_time = local_time;
	      
	      utc = gmtime (&local_time);
	      sprintf (cptr + 1, "%04d-%02d-%02d",
		       utc->tm_year + 1900,
		       utc->tm_mon + 1,
		       utc->tm_mday);
	      sprintf (cptr + 12, "%02d:%02d:%02d",
		       utc->tm_hour, utc->tm_min, utc->tm_sec);
	      cptr[11] = sep1;
	      cptr[20] = sep2;
	  }
	  tstamp_col = -1;
      }
  }
  return local_time;
}

int sleepfor (int nsec)
{
    struct timespec nsecond;

    nsecond.tv_sec = nsec;
    nsecond.tv_nsec = 0;

    return nanosleep (&nsecond, NULL);
}


/* Format a JSON document
   It would be better to do this in the loader threads but
   we need the key formatted to that we know the queue number */
int json_format (char *line_in, char *line_out, char **keys, int ncols)
{
    int   i, len = strlen (line_in);
    char *jl_ptr = line_out;
    
    for (i = 0; i < ncols; i++)
    {
	jl_ptr += json_lookup (line_in, len, keys[i], jl_ptr);
	*jl_ptr++ = '|';
	*jl_ptr = '\0';
    }	    			     
    *jl_ptr++ = '\n';
    *jl_ptr = '\0';
    len = jl_ptr - line_out;
    
    return len;
}

void send_cmd_queue (db_thread *t, hdr_t *hdr, node *n, int queue)
{
    global    *g = t->globals;
    int        err = 0;

    if (g->verbose > 1)
	printf ("%s: Sending message to server %s, queue %d fd %d\n",
		t->name, n->name, queue, n->pipelist[queue]);
    err = write_pipe (t, n->pipelist[queue], (char *)hdr, sizeof (hdr_t));

    if (err && g->verbose > 1)
	printf ("%s: Could not send message to server %s, queue %d\n",
		t->name, n->name, queue);
}

void send_cmd_node (db_thread *t, hdr_t *hdr, node *n)
{
    int        queue;

    for (queue = 0; queue < n->nqueues; queue++)
	send_cmd_queue (t, hdr, n, queue);
}

void send_cmd_all (db_thread *t, hdr_t *hdr)
{
    global    *g = t->globals;
    int        server;
    node      *n;

    for (server = 0; server < g->nservers; server++)
    {
	n = &(g->server_list[server]);
	send_cmd_node (t, hdr, n);
    }
}

void send_flush_thread (db_thread *t, node *n, int queue)
{
    hdr_t      hdr;

    hdr.len = CMD_HDR;
    hdr.dev_id = CMD_FLUSH;
    send_cmd_queue (t, &hdr, n, queue);
}

void send_flush_node (db_thread *t, node *n)
{
    hdr_t      hdr;

    hdr.len = CMD_HDR;
    hdr.dev_id = CMD_FLUSH;
    send_cmd_node (t, &hdr, n);
}

void send_flush_all (db_thread *t)
{
    global    *g = t->globals;
    int        server;
    node      *n;

    for (server = 0; server < g->nservers; server++)
    {
	n = &(g->server_list[server]);
	send_flush_node (t, n);
    }
}

void shutdown_all (db_thread *t)
{
    db_thread *tshut;
    global    *g = t->globals;
    int        server, i, queue;
    hdr_t      hdr;
    node      *n;

    hdr.len = CMD_HDR;
    hdr.dev_id = CMD_TERMINATE;
    hdr.ts_id = 0;
    
    for (server = 0; server < g->nservers; server++)
    {
	int   err = 0;
	
	n = &(g->server_list[server]);
	send_cmd_node (t, &hdr, n);
	
	for (queue = 0; queue < n->nqueues; queue++)
	    pthread_kill (n->threadlist[queue]->thread, SIGINT);
	    
	/* Should keep a list of fds that need shutting down */
	if (n->storagefd > 0)
	    err += write (n->storagefd, &hdr, sizeof (hdr_t)) > 0 ? 0 : 1;
	if (n->rewritefd > 0)
	    err = write (n->rewritefd, &hdr, sizeof (hdr_t)) > 0 ? 0 : 1;
	if (n->provisionfd > 0)
	    err = write (n->provisionfd, &hdr, sizeof (hdr_t)) > 0 ? 0 : 1;
	if (err && g->verbose > 1)
	    printf ("%s: Could not send close messages\n", t->name);
    }
    if ((tshut = find_thread (T_STATUS)) != NULL)
	tshut->treturn = 1;
    if (g->input && (tshut = find_thread (T_DIR_READER)) != NULL)
    {
	tshut->treturn = 1;
	pthread_kill (tshut->thread, SIGINT);
    }
    if (g->input_dir)
    {
	for (i = 0; i < g->in_threads; i++)
	{
	    tshut = g->dir_reader[i];
	    tshut->treturn = 1;	
	    pthread_kill (tshut->thread, SIGINT);
	}
    }
}


int write_pipe (db_thread *t, int piped, char *buf, int len)
{
    int bytes;

    if (t->treturn != 0 || end_process)
	return -1;
    
    if ((bytes = write (piped, buf, len)) != len)
    {
	if (t->treturn != 1)
	{
	    printf ("%s: Error writing to pipe %d, bytes = %d\n",
		    t->name, piped, bytes);
	    t->treturn = -1;
	    return -1;
	}
    }

    return bytes == len ? 0 : 1;
}

/* This puts a data buffer */
int put_data (db_thread *t, char *buffer, int len)
{
    int fd;
    int nbytes = 0;

    t->nput++;
    t->puts_this_flush++;

    if (t->globals->pass_through == 0)
    {
	fd = open ("/tmp/debug.unl", O_RDWR|O_CREAT|O_APPEND, 00666);
	if (fd == -1)
	    return -1;
	nbytes = write (fd, buffer, len);
	close (fd);
	return nbytes;
    }
    if (t->putbuf_file)
    {
	fd = open (t->putbuf_file + 5, O_RDWR|O_CREAT|O_APPEND, 00666);
	if (fd == -1)
	    return -1;
	nbytes = write (fd, buffer, len);
	close (fd);
    }
    else
    {
	if (t->globals->pass_through == 0)
	    return 0;
    
	db_put_data (t, buffer);
	nbytes = len;
    }
    
    return nbytes;
}

int put_buffer (db_thread *t)
{
    int res = 0;

    if (*t->buffer)
    {
	res = put_data (t, t->buffer, t->bptr - t->buffer);
	*(t->bptr = t->buffer) = '\0';
    }

    return res;
}


void flush_data (db_thread *t)
{
    t->nflush++;

    if (t->putbuf_file)
    {
	if (t->globals->pass_through != 0)
	{
	    db_put_data (t, t->putbuf_file);
	    /* TODO: If there was an error on the flush it may be better
	       to save this file for later examination. */
	    unlink (t->putbuf_file + 5);
	}
    }

    if (t->globals->pass_through != 0)
	db_flush_data (t);

    t->puts_this_flush = 0;
}
