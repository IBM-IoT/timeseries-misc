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
 *  Title            : threads.c
 *  Author           : Cosmo@uk.ibm.com
 *  Description      : Thread processing routines for Parallel TimeSeries Loader
 *
 *  Version          : 1.0	2015-04-01 Split out from process.c
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

#include "mi_search.h"

#include "defs.h"


/* Don't move the keycpy, keycmp or inputlen functions to another
   source file. The compiler optimiser will inline them for speed */
void keycpy (char *key, char *line, int ncols, char ifs)
{
    int   i;
    char *k = key, *l = line;

    for (i = 0; i < ncols; k++, l++)
    {
	*k = *l;
	if (*l == ifs)
	    i++;
    }
    *(k - 1) = '\0';
}

int keycmp (char *key, char *line, int ncols, char ifs)
{
    int   i;
    char *k = key, *l = line;

    /* First time round previous key will be empty */
    if (*key == '\0')
	return -1;
    
    /* Look for a compressed key like: '|||' */
    for (l = line, i = 0; *l && i < ncols; l++)
    {
	if (*l == ifs)
	    i++;
	else
	    break;
    }
    if (i == ncols)
	return 0;
    
    for (k = key, l = line, i = 0; i < ncols; k++, l++)
    {
	if (*k != *l)
	    break;
	if (*l == ifs)
	    i++;
    }
    if (i == ncols)
	return 0;

    return 1;
}

/* Get the length up to the \n - fix or reject other non-ASCII */
int inputlen (unsigned char *in, int fix, int *reject)
{
    int count, rv = 0, fixed = 0;
    
    for (count = 0; rv == 0 && *in != '\n'; count++, in++)
    {
	if ((*in < ' ') || (*in == '\\'))
	{
	    if (fix)
	    {
		fixed++;
		*in = '?';
	    }
	    else
		rv = -1;
	}
	if (count >= LINE_SIZE)
	    rv = -1;
    }
    if (fixed > 0 || rv == -1)
	(*reject)++;
	    
    return (rv == 0 ? count : rv);
}

void *status (void *v)
{
    db_thread      *t = v, *reader_thread = NULL;
    global         *g;
    int             seconds, last_report, last_count, nrec, slept, i;

    g = t->globals;
    
    printf ("Status thread reporting every %d seconds\r",
	    g->status_update);
    
    fflush (stdout);

    nrec = 0;
    
    /* Although we may be giving a status update every N seconds we only
       sleep for 1 second so we can check the status flag to see if we
       have been told to exit */
    for (last_count = last_report = seconds = 0;
	 sleepfor (1) == 0;
	 seconds++)
    {
	slept = seconds - last_report;
	
	if (t->treturn != 0)
	    break;
	
	if (slept > g->status_update)
	{
	    struct timeval last_put = {0,0}, last_put_tv = {0,0},
		     last_flush = {0,0}, last_flush_tv = {0,0};
	    
	    for (i = 0; i < thread_block_size; i++)
	    {
		db_thread *l;

		l = tlist[i];
			   
		if (l && l->type == T_LOADER)
		{
		    if (timercmp (&(l->last_put_tv), &last_put_tv, >))
		    {
			last_put = l->put_elapsed;
			last_put_tv = l->last_put_tv;
		    }
		    if (timercmp (&(l->last_flush_tv), &last_flush_tv, >))
		    {
			last_flush = l->flush_elapsed;
			last_flush_tv = l->last_flush_tv;
		    }
		    /* Status thread holds MAX times */
		    if (timercmp (&(l->put_elapsed), &t->put_elapsed, >))
			t->put_elapsed = l->put_elapsed;
		    if (timercmp (&(l->flush_elapsed), &t->flush_elapsed, >))
			t->flush_elapsed = l->flush_elapsed;
		}
	    }
	    
	    if (g->input)
	    {
		if (reader_thread == NULL)
		    reader_thread = find_thread (T_READER);
		if (reader_thread)
		    nrec = reader_thread->nint;
	    }
	    
	    if (g->input_dir)
	    {
		for (nrec = i = 0; i < g->in_threads; i++)
		    nrec += g->dir_reader[i]->nint;
	    }
	    
	    if (nrec > 0)
	    {
		printf ("At input record %d average rate is %d - last put %ld.%06d, last flush %ld.%06d                \r",
			nrec, (nrec - last_count)/slept,
			last_put.tv_sec, (int) last_put.tv_usec,
			last_flush.tv_sec, (int) last_flush.tv_usec);
		fflush (stdout);
	    }
	    last_report = seconds;
	    last_count = nrec;
	}
    }

    if (g->input && reader_thread)
	nrec = reader_thread->nint;

    if (g->input_dir)
    {
	for (nrec = i = 0; i < g->in_threads; i++)
	    nrec += g->dir_reader[i]->nint;
    }
    
    slept = seconds - last_report;

    printf ("At input record %d average rate is %d                \r",
	    nrec, (nrec - last_count)/(slept == 0 ? 1 : slept));
    fflush (stdout);
    
    if (t->globals->verbose > 1)
	printf ("%s: Exiting with status %d          \n", t->name, t->treturn);

    fflush (stdout);

    if (t->globals->nthreads > 0)
	pthread_exit (&t->treturn);

    return &(t->treturn);
}

void *provision (void *v)
{
    db_thread *t = v, *l;
    hdr_t     *hdr;
    ts_data   *d;
    global    *g;
    node      *n = NULL;
    char       *line, line_buf[LINE_SIZE+sizeof (hdr)], key[LINE_SIZE];
    int        inlen, least_full_cont, least_full_loader;

    g = t->globals;

    n = &(g->server_list[t->connection->current_server]);
    
    t->treturn = 0;
    
    if (db_connect (t, t->connection->current_server, T_PROVISION) == 0)
    {
	printf ("%s: Provsioning thread %d could not connect to database <%s@%s>\n",
		t->name, t->tid, g->database, n->name);
	goto finish;
    }

    hdr = (hdr_t *) line_buf;
    line = (char *) (hdr + 1);

    for ( ; ; )
    {
	inlen = get_message (t, hdr, line);
	
	if (inlen == CMD_HDR)
	{
	    if (hdr->dev_id == CMD_TERMINATE)
		goto finish;
	    else
		continue;
	}
	
	if (line[inlen-1] == '\n')
	    line[--inlen] = '\0';
    
	keycpy (key, line, g->pk_column_count, g->ifs);

	if (g->new_ts)
	{
	    int        i, loader, spec_len = 0, min_refs, min_ncontainers;
	    char      *ts_spec, *cname = NULL;
		
	    /* If container is not specified use the one
	       with fewest TimeSeries */
	    if (strstr (g->new_ts, "container(") == NULL)
	    {
		min_refs = n->threadlist[0]->container_list[0]->refs;
		least_full_loader = 0;
		least_full_cont = 0;
		for (loader = 0; loader < n->nqueues; loader++)
		{
		    l = n->threadlist[loader];
		    for (i = 0; i < l->ncontainers; i++)
			if (l->container_list[i]->refs < min_refs)
			{
			    least_full_loader = loader;
			    least_full_cont = i;
			    min_refs = l->container_list[i]->refs;
			}
		}
		l = n->threadlist[least_full_loader];
		cname = l->container_list[least_full_cont]->name;
		spec_len = strlen (cname) + strlen (g->new_ts) + 13;
		ts_spec = (char *) malloc (spec_len);
		sprintf (ts_spec, "%s,container(%s)", g->new_ts, cname);
	    }
	    else
		ts_spec = g->new_ts;

	    d = n->ts_data_list[hdr->dev_id];

	    if ((d = insert_new_ts (t, (unsigned char *)key, ts_spec, d)) != NULL)
	    {
		/* If we picked the container then we know what loader
		   to send the data to */
		if (cname &&
		    strcmp (cname, n->container_list[d->container]->name) == 0)
		    loader = least_full_loader;
		else
		{
		    char      cmd_buf[255];
		    hdr_t    *cmd;

		    /* Find the loader for this container */
		    cname = n->container_list[d->container]->name;
		    for (loader = 0; loader < n->nqueues; loader++)
		    {
			l = n->threadlist[loader];
			for (i = 0; i < l->ncontainers; i++)
			    if (strcmp (l->container_list[i]->name, cname) == 0)
				goto found_loader;
		    }
		    /* If we get here then a new container has been allocated.
		       Look for the loader managing the fewest containers */
		    min_ncontainers = n->threadlist[0]->ncontainers;
		    least_full_loader = 0;
		    for (loader = 0; loader < n->nqueues; loader++)
		    {
			l = n->threadlist[loader];
			if (l->ncontainers < min_ncontainers)
			{
			    least_full_loader = loader;
			    min_ncontainers = l->ncontainers;
			}
		    }
		    loader = least_full_loader;

		    /* Tell the loader about the new container */
		    cmd = (hdr_t *) cmd_buf;
		    strcpy ((char *) (cmd + 1), cname);
		    cmd->len = strlen (cname) + sizeof (hdr_t);
		    cmd->dev_id = CMD_NEW_CONTAINER;
		    cmd->ts_id = d->container;
		    if (write_pipe (t, n->pipelist[loader], cmd_buf,
				    cmd->len + sizeof (hdr_t)) == -1)
			goto finish;
		}

	    found_loader:

		if (write_pipe (t, n->pipelist[loader], line_buf,
				hdr->len + sizeof (hdr_t)) == -1)
		    goto finish;
	    }
	    else
		log_reject (g->rejects, line);	    

	    if (spec_len)
		free (ts_spec);
	}
    }

finish:

    if (g->verbose > 1)
	printf ("%s: Exiting with status %d          \n", t->name, t->treturn);    
    if (g->nthreads > 0)
	pthread_exit (&t->treturn);

    return &(t->treturn);
}

void *reader (void *v)
{
    db_thread *t = v;
    FILE      *fin = NULL;
    unsigned char  *line, json_line_buf[LINE_SIZE], next_line_buf[LINE_SIZE];
    unsigned char   key[LINE_SIZE], *json_line, *next_line, *line_buf;
    int        queue, namelen;
    hdr_t     *hdr;
    ts_data   *d;
    global    *g;
    node      *n = NULL;
    char      *filename = NULL, *dirname = NULL, *inputname;
    char      *in_name = NULL, *proc_name = NULL, *comp_name = NULL;

    t->treturn = 0;
    
    g = t->globals;

    if (db_connect (t, t->connection->current_server, T_READER) == 0)
    {
	printf ("%s: Reader thread %d could not connect to database <%s@%s>\n",
		t->name, t->tid, g->database, n->name);
	goto finish;
    }
      
    filename = (char *) malloc (FILENAME_LEN + 1);
    
    if (g->json_ncols > 0)
    {
	line = json_line = json_line_buf + (sizeof (hdr_t));
	line_buf = json_line_buf;
    }
    else
    {
	line = next_line = next_line_buf + (sizeof (hdr_t));
	line_buf = next_line_buf;
    }
    hdr = (hdr_t *)line_buf;

top_loop:
    
    if (t->type == T_DIR_READER)
    {
	if (dirname == NULL)
	{
	    dirname = (char *) malloc (strlen (g->input_dir)
				       + strlen (INPUT_DIR) + 2);
	    sprintf (dirname, "%s/%s", g->input_dir, INPUT_DIR);
	}

	/* TODO: We only process files from the Input directory - it could be
	   argued that the Process directory should be scanned first to clean
	   up from a previously interupted run. Easier to make this part of
	   the external processing architecture, although the decission to
	   use the correct flush-flag (5 - overwrite) could be automated.
	   See also the TODO regarding latest time stamps to prevent
	   duplciates. */
	
	if (next_file_in_dir (dirname, "*.tsl", filename) == 0)
	{
	    namelen = strlen (g->input_dir) + strlen (filename);
	    in_name = (char *) malloc (namelen + strlen (INPUT_DIR) + 3);
	    proc_name = (char *) malloc (namelen + strlen (PROCESS_DIR) + 3);
	    comp_name = (char *) malloc (namelen + strlen (COMPLETE_DIR) + 3);

	    sprintf (in_name, "%s/%s/%s", g->input_dir, INPUT_DIR, filename);
	    sprintf (proc_name, "%s/%s/%s", g->input_dir, PROCESS_DIR, filename);
	    sprintf (comp_name, "%s/%s/%s", g->input_dir, COMPLETE_DIR, filename);
	    
	    if (g->verbose)
		printf ("%s: Found <%s>\n", t->name, in_name);
	    
	    if (rename (in_name, proc_name) != 0)
	    {
		if (g->verbose)
		    printf ("%s: Someone else already picked up <%s>\n",
			    t->name, in_name);
		fin = NULL;
		goto clean_up;
	    }
	    inputname = proc_name;
	}
	else
	{
	    /* With this flag set we exit at the end of all input files */
	    if (g->one_shot)
		t->treturn = 1;
	    
	    /* Told to shutdown */
	    if (t->treturn == 1)
		goto clean_up;
	    
	    sleepfor (1);

	    /* Otherwise wait for more input */
	    goto top_loop;
	}
    }
    else
	inputname = g->input;

    if (inputname[0] == '-' && inputname[1] == '\0')
    {
	if (g->verbose)
	    printf ("%s: Reading from STDIN\n", t->name);
	fin = stdin;
    }
    else
    {
	if ((fin = fopen (inputname, "r")) == NULL)
	{
	    printf ("%s: Cannot open <%s>\n", t->name, inputname);
	    t->treturn = -1;
	    goto finish;
	}
	if (g->verbose)
	    printf ("%s: Opening <%s>\n", t->name, inputname);
    }

    queue = 0;
    key[0] = '\0';
    while (fgets ((char *)next_line, LINE_SIZE - (sizeof (hdr_t)), fin) != NULL)
    {
	int len;

	if (g->json_ncols > 0)
	    hdr->len = json_format ((char *)next_line, (char *)json_line,
				    g->json_key_names, g->json_ncols);
	else
	    hdr->len = inputlen (next_line, g->fix_non_ascii, &(t->rejects));

	if (hdr->len == -1)
	{
	    if (g->verbose)
		printf ("%s: Rejected line <%s>\n", t->name, next_line);
	    log_reject (g->rejects, (char *)next_line);	    
	    continue;
	}
	if (hdr->len == 1 && *next_line == '\n')
	    continue;
	
	t->nint++;
	
	if (g->nthreads > 0)
	{
	    if (keycmp ((char *)key, (char *)line, g->pk_column_count, g->ifs) != 0)
	    {
		keycpy ((char *)key, (char *)line, g->pk_column_count, g->ifs);
		
		d = hash_search (&g->pk_hash, (char *)key);

		/* A missing key means their was no base table entry when the
		   hash map was built. */
		
		if (d == NULL)
		{
		    /* First check to see if has been inserted into the base
		       table while we were running */		    
		    if ((d = load_key_info (t, key, NULL)) == NULL)
		    {
			int    server, min_rows;
			node  *least_busy;
			
			if (g->verbose)
			    printf ("%s: Not in hash list <%s>\n", t->name, key);

			/* This is per-node so need to choose a node.
			   TODO: Something more dynamic than just the number
			   of rows might be better. */
			least_busy = &(g->server_list[0]);
			min_rows = least_busy->nrows;
			for (server = 0; server < g->nservers; server++)
			{
			    n = &(g->server_list[0]);
			    if (min_rows > n->nrows)
			    {
				min_rows = n->nrows;
				least_busy = n;
			    }
			}
			n = least_busy;

			/* Must allocate data structures up front in
			   this thread to prevent multiple updaters
			   on the hash lists */
			d = store_ts_data (t, 0, n, NULL, (char *)key, server, 0, NULL);
			hdr->dev_id = d->id;

			hash_insert (&g->pk_hash, d->key, d);
			
			if (n->provision_thread)
			{
			    len = hdr->len + sizeof (hdr_t);
			    if (write_pipe (t, n->provisionfd,
					    (char *)line_buf, len) == -1)
				goto finish;
			}			
			continue;
		    }
		}
		hdr->dev_id = d->id;
		hdr->ts_id = d->ts_id;
		n = &(g->server_list[find_server(t, d->nodeid_now)]);

		if (t->mod_queue)
		    queue = t->nint % g->nthreads;
		else
		    queue = n->container_list[d->container]->queue;
		
		if (g->verbose > 2)
		    printf ("%s: Found <%s> in container %d on queue %d\n",
			    t->name, key, d->container, queue);
	    }
	}
	else
	    hdr->dev_id = 0;

	/* TODO: In order to prevent duplicates it would be possible to select
	   the getlastelem().tstamp from the TimeSeries on start up and
	   compare this to the incomimg data. */
	    
	/* The length and container_id is sent together with the
	   data in one buffer as a single atomic operation.
	   If not, multi-threaded writes will break. */
	len = hdr->len + sizeof (hdr_t);

	if (write_pipe (t, n->pipelist[queue], (char *)line_buf, len) == 0)
	{
	    if (g->verbose > 1)
		printf ("%s: wrote %d bytes [%d:%d]\n", t->name, len, hdr->len, hdr->dev_id);
	}
	else
	    /* There is a distinction here bewteen finish and clean_up.
	       If we fail on the write, midway through processing a file,
	       then goto finish will leave the file in the Process directory
	       indicating that it was not loaded completely. */
	    goto finish;
    }

clean_up:
    
    /* The directory reader loops forever (unless --one-shot is set) */
    if (t->type == T_DIR_READER)
    {
	if (fin)
	{
	    fclose (fin);
	    fin = NULL;

	    if (comp_name)
	    {
		if (g->verbose)
		    printf ("%s: Moving <%s> to <%s>\n", t->name, proc_name, comp_name);
		
		if (rename (proc_name, comp_name) != 0)
		{
		    perror (comp_name);
		    t->treturn = -1;
		    goto finish;
		}
	    }
	}
	if (comp_name)
	    free (comp_name);
	comp_name = NULL;
	if (in_name)
	    free (in_name);
	in_name = NULL;
	if (proc_name)
	    free (proc_name);
	proc_name = NULL;

	if (end_process == 0 && t->treturn == 0)
	    goto top_loop;
    }

    if (g->verbose)
	printf ("%s: End of input stream\n", t->name);

    t->treturn = 1;
    
finish:

    if (g->verbose > 1)
	printf ("%s: Exiting with status %d          \n", t->name, t->treturn);    
    
    if (g->nthreads > 0)
	pthread_exit (&t->treturn);

    return &(t->treturn);
}

void *loader (void *v)
{
    db_thread *t = v;
    int        i, last_flush, in_tx;
    char      *ts_start;
    int        last_report, last_inlen;
    int        year = 0, month = 0, day = 0;
    char      *line, json_line[LINE_SIZE], next_line[LINE_SIZE];
    FILE      *fin = NULL;
    node      *n;
    global    *g;
    ts_data   *d = NULL;

    g = t->globals;
    n = &(g->server_list[t->connection->current_server]);
    
    t->treturn = 0;
    
    if (g->nthreads > 0)
	fin = NULL;
    else
    {
	if (g->input[0] == '-' && g->input[1] == '\0')
	{
	    fin = stdin;
	}
	else
	{
	    char *fname = (char *) malloc (strlen (g->input)+16);
	    sprintf (fname, g->input, t->tid);
	    if ((fin = fopen (fname, "r")) == NULL)
	    {
		printf ("%s: Cannot open <%s>\n", t->name, fname);
		goto finish;
	    }
	    if (g->verbose)
		printf ("%s: Opening <%s>\n", t->name, fname);
	    free (fname);
	}
	if (g->status_update > 0)
	{
	    printf ("Warning - no status updates in single process mode\r");
	    fflush (stdout);
	}
    }

    if (g->putbuf_dir)
    {
	char prefix[] = "PutBuf_";
	int len = 5 + strlen (g->putbuf_dir) + sizeof (prefix)
	    + strlen (t->name) + 1;
	
	t->putbuf_file = (char *) malloc (len);
	sprintf (t->putbuf_file, "FILE:%s/%s%s",
		 g->putbuf_dir, prefix, t->name);
	unlink (t->putbuf_file + 5);
    }
    
    if (db_connect (t, t->connection->current_server, T_LOADER) == 0)
    {
	printf ("%s: Loader thread %d could not connect to database <%s@%s>\n",
		t->name, t->tid, g->database, n->name);
	goto finish;
    }
  
    if (table_attach (t) < 0)
    {
	printf ("%s: Loader thread %d could not attach to loader segment <%s>\n",
		t->name, t->tid, n->handle);
	goto release;
    }

    if (g->verbose)
	printf ("%s: Loader thread %d connected to database <%s@%s> as handle <%s>\n",
	    t->name, t->tid, g->database, n->name, n->handle);

    i = 0;
    last_flush = last_report = 0;

    t->buffer = (char *) malloc (g->put_buffer);
    *(t->bptr = t->buffer) = '\0';
    
    if (g->lock_containers)
	t->container_locks = 1;

    /* Need a start time for the flush interval logic */
    gettimeofday (&(t->last_flush_tv), NULL);

    last_inlen = LINE_SIZE - 2;
    for (line = next_line, in_tx = 0; ; )
    {
	int    inlen, container_id = -1, do_flush;
	hdr_t  hdr_d, *hdr = &hdr_d;

	do_flush = 0;

	if (fin)
	{
	    if (g->verbose > 1)
		printf ("%s: Reading from input stream\n", t->name);
	    if (fgets (next_line, LINE_SIZE, fin) == NULL)
		break;
	    if (g->json_ncols > 0)
	    {
		inlen = json_format (next_line, json_line, g->json_key_names,
				   g->json_ncols);
		line = json_line;
	    }
	    else
	    {
		inlen = inputlen ((unsigned char *)next_line, g->fix_non_ascii, &(t->rejects));
		line = next_line;
	    }
	}
	else
	{
	    memset (line, 0, last_inlen + 2);

	    inlen = get_message (t, hdr, line);

	    if (inlen == CMD_HDR || hdr->dev_id < 0)
	    {
		container **new_list;
		
		switch (hdr->dev_id)
		{
		case CMD_TERMINATE:
		    goto terminate;
		    break;
	    
		case CMD_NEW_CONTAINER:
		    new_list = (container **) calloc (t->ncontainers + 1,
						      sizeof (container *));
		    for (i = 0; i < t->ncontainers; i++)
			new_list[i] = t->container_list[i];		
		    new_list[t->ncontainers] = store_container (line, n);
		    free (t->container_list);
		    t->container_list = new_list;
		    t->ncontainers++;
		    continue;
		    break;
		    
		case CMD_FLUSH:
		    do_flush = 2;
		    goto flush;
		    break;

		default:
		    break;
		}
	    }
	}

	last_inlen = inlen;
	
	d = n->ts_data_list[hdr->dev_id];

	t->nint++;
	
	if (line[inlen-1] == '\n')
	    line[--inlen] = '\0';
		
	if (g->to_utc)
	    (void) parse_line (line, inlen, g->pk_column_count + 1, g->ifs,
				 &(d->last_tstamp),
				 &ts_start, &year, &month, &day);

	if (g->skip_cols != -1)
	{
	    inlen = remove_repeats (line, inlen, d, g->ifs,
				    g->pk_column_count + 1, g->skip_cols,
				    g->remove_all_nulls, g->repeat_marker);
	    if (inlen == 0)
	    {
		t->nallnulls++;
		continue;
	    }
	    else if (inlen == -1)
	    {
		if (g->verbose)
		    printf ("ERROR in <%s>\n", line);
		continue;
	    }

	    line[inlen] = '\0';
	}
	if (inlen > last_inlen)
	    last_inlen = inlen;
	
	switch (g->pass_through)
	{
	case 1:
	    printf ("%s\n", line);
	    continue;
	    break;
	case 2:
	    fprintf (stderr, "%s\n", line);
	    continue;	    
	    break;
	}

	in_tx++;
	
	container_id = d->container;
	n->container_list[container_id]->lock = 1;

	if (t->container_locks && hdr->dev_id >= 0 && n->ts_data_list)
	{
	    /* Can't use TSL_Commit with container locks */
	    if (g->txsize > 0 && in_tx >= g->txsize)
		do_flush = 1;
	}

	if (((t->bptr - t->buffer) + inlen) >= (g->put_buffer - 20))
	    put_buffer (t);

	if ((t->nput - last_flush) >= g->put_count)
	    do_flush = 1;
	
    flush:

	if (do_flush)
	{
	    put_buffer (t);
	    flush_data (t);
	    last_flush = t->nput;
	    in_tx = 0;
	}
	
	if (do_flush == 2)
	    continue;
	
	if (g->meta_join)
	{
	    long ts_id;

	    /* Is TimeSeries being relocated */
	    if (hdr->ts_id != -1)
		ts_id = hdr->ts_id;
	    else
		ts_id = d->ts_id;
	    
	    inlen = sprintf (t->bptr, "%ld|%s", ts_id, ts_start);
	}
	else
	    memcpy (t->bptr, line, inlen);	
	t->bptr += inlen;
	*t->bptr++ = '\n';
	*t->bptr = '\0';
    }

terminate:
    
    /* Anything left ? */
    if (t->nint > 0)
    {
	put_buffer (t);
	flush_data (t);
    }
    
release:
    
    tsl_release (t);
    
    db_disconnect (t);
    
finish:
    if (g->verbose > 1)
	printf ("%s: Exiting with status %d          \n", t->name, t->treturn);

    if (g->nthreads > 0)
	pthread_exit (&t->treturn);

    return &(t->treturn);
}

void *init (void *v)
{
    db_thread *t = v;
    node      *n;
    global    *g;

    g = t->globals;
    n = &(g->server_list[t->connection->current_server]);
    
    if (db_connect (t, t->connection->current_server, T_MAIN) == 0)
    {
	printf ("%s: Could not connect to database <%s@%s> for initialisation\n",
		t->name, g->database, n->name);
	exit (1);
    }

    t->treturn = 1;
    if (table_init (t) < 0)
    {
	printf ("%s: Could not initialise loader segment for <%s@%s:%s.%s>\n",
		t->name, g->database, n->name,
		g->table, g->column);
	t->treturn = 0;
    }
    if (n->handle)
	printf ("%s: Initialised loader with handle <%s>\n",
		t->name, n->handle);
    
    db_disconnect (t);

    if (g->nthreads > 0)
	pthread_exit ((void *)(long)t->treturn);

    return (void *)(long)(t->treturn);
}

void *hashlists (void *v)
{
    db_thread *t = v;
    node      *n;
    global    *g;

    g = t->globals;
    n = &(g->server_list[t->connection->current_server]);

    if (n->nrows == 0)
	n->ts_data_list_size = MIN_TS_DATA_SIZE * 2;
    else
	n->ts_data_list_size = n->nrows + MIN_TS_DATA_SIZE;
    
    n->ts_data_list = (ts_data **) calloc (n->ts_data_list_size, sizeof (ts_data *));

    if (db_connect (t, t->connection->current_server, T_MAIN) == 0)
    {
	printf ("%s: Could not connect to database <%s@%s> to get hash lists\n",
		t->name, g->database, n->name);
	exit (1);
    }
    
    get_containers (t);
    
    get_keys (t);
    
    db_disconnect (t);

    t->treturn = 1;
    
    if (g->nthreads > 0)
	pthread_exit ((void *)(long)t->treturn);

    return (void *)(long)(t->treturn);
}

void *rewrite (void *v)
{
    db_thread *t = v;
    node      *n;
    global    *g;
    int        nrewriten = 0, nrows, inlen;
    ts_data   *d;
    long       old_id, new_id;
    char       key[IDENT_LEN];
    char       line[LINE_SIZE];
    hdr_t      s_hdr, *hdr = &s_hdr;

    g = t->globals;
    n = &(g->server_list[t->connection->current_server]);
    
    t->treturn = 0;
    
    if (db_connect (t, t->connection->current_server, T_REWRITE) == 0)
    {
	printf ("%s: Could not connect to database <%s@%s> to start re-writer\n",
		t->name, g->database, n->name);

	t->treturn = -1;
	goto end_thread;
    }
    if (table_attach (t) < 0)
    {
	printf ("%s: Re-Write thread %d could not attach to loader segment <%s>\n",
		t->name, t->tid, n->handle);
	t->treturn = -1;
	goto end_thread;
    }

    while ((old_id = db_next_displaced (t, key)))
    {
	char  container[IDENT_LEN+1];
	short hertz = 0;
	
	/* This will increment the sequence number that links the device
	   and metadevice tables. It also gets the necesssary TimeSeries
	   info to redirect the data to a new queue. */
	new_id = db_next_device (t, container, hertz);
	
	d = hash_search (&g->pk_hash, key);
	
	if (d && g->verbose > 0)
	    printf ("%s: Moving device %s as id %ld on node %d to new id %ld on %d\n", t->name,
		    d->key, old_id, d->nodeid_now, new_id, n->id);
	
	d->status = MOVE_BASE_ROWS;
	nrows = db_move_ts (t, old_id, new_id, find_server (t, d->nodeid_now));
	if (g->verbose > 0)
	    printf ("%s: Moved %d rows to node %d\n", t->name, nrows, n->id);
	
	if (nrows > 0)
	{
	    t->nint += nrows;
	    nrewriten++;
	}
	
	inlen = get_message (t, hdr, line);

	if (inlen == CMD_HDR && hdr->dev_id == CMD_TERMINATE)
	    break;	
	
    }
    t->treturn = nrewriten;

end_thread:

    db_disconnect (t);
    
    if (g->nthreads > 0)
	pthread_exit ((void *)(long)t->treturn);

    return (void *)(long)(t->treturn);
}


void *storage (void *v)
{
    db_thread *t = v;
    hdr_t      s_hdr, *hdr = &s_hdr;
    node      *n;
    global    *g;
    int        nstored = 0, inlen, year, month, day;
    char       line[LINE_SIZE], *ts_start;
    ts_data   *d;

    g = t->globals;
    n = &(g->server_list[t->connection->current_server]);
    
    t->treturn = 0;
    
    if (db_connect (t, t->connection->current_server, T_STORAGE) == 0)
    {
	printf ("%s: Could not connect to database <%s@%s> to start storage thread\n",
		t->name, g->database, n->name);

	t->treturn = -1;
	goto end_thread;
    }
    if (table_attach (t) < 0)
    {
	printf ("%s: Storage thread %d could not attach to loader segment <%s>\n",
		t->name, t->tid, n->handle);
	goto end_thread;
    }
    for ( ; ; )
    {
	char *vals;
	
	inlen = get_message (t, hdr, line);

	if (inlen == CMD_HDR && hdr->dev_id == CMD_TERMINATE)
	    break;
	
	d = n->ts_data_list[hdr->dev_id];
	
	parse_line (line, inlen, g->pk_column_count + 1, g->ifs,
		    g->to_utc ? &(d->last_tstamp) : NULL,
		    &ts_start, &year, &month, &day);
	
	*(ts_start - 1) = '\0';
	vals = strchr (ts_start + 12, '|');
	*vals++ = '\0';
	
	db_insert_storage (t, line, ts_start, vals); 
	
	t->nint++;
    }
    
    t->treturn = nstored;

end_thread:
    
    db_disconnect (t);

    if (g->verbose > 1)
	printf ("%s: Exiting with status %d          \n", t->name, t->treturn);
    
    if (g->nthreads > 0)
	pthread_exit ((void *)(long)t->treturn);

    return (void *)(long)(t->treturn);
}
