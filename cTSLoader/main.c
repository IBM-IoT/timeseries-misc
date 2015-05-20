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
 *  Title            : main.c
 *  Author           : Cosmo@uk.ibm.com
 *  Description      : Main driver for Parallel TimeSeries Loader
 *
 *  Version          : 2.0	2014-03-06 Extensively re-written for 6.00.xC2
 *
 ****************************************************************************
 */

#define _GNU_SOURCE
#include <sys/time.h>
#ifdef HAVE_prctl
#include <sys/prctl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <libgen.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <stddef.h>
#include <fnmatch.h>
#include <signal.h>

#include "mi_search.h"

#include "defs.h"

/* These are globals so that the thread can be found when the context has
   been misplaced. Don't play fast and loose with them outside the main
   thread. */

/* How many threads in the initial list - also growth size */
#define         BLOCK_SIZE    128

int thread_total = 0;
int thread_block_size = 0;
db_thread **tlist = NULL;
/* Not everyone has access to g->verbose */
int verbose = 0;

int TSL_Init (db_thread *m);
int TSL_Load (db_thread *m);
int TSL_ShutDown (db_thread *m);

#ifndef timeradd
void timeradd (struct timeval *a, struct timeval *b, struct timeval *res)
{
    res->tv_sec = a->tv_sec + b->tv_sec;
    res->tv_usec = a->tv_usec + b->tv_usec;
    if (res->tv_usec > 999999)
    {
        res->tv_usec -= 999999;
        res->tv_sec++;
    }
}
#endif

#ifndef timersub
void timersub (struct timeval *a, struct timeval *b, struct timeval *res)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  if (a->tv_usec < b->tv_usec)
  {
      res->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
      res->tv_sec--;
  }
  else
      res->tv_usec = a->tv_usec - b->tv_usec;

}
#endif

/* Man page for real timeradd doesn't specify behaviour of
   timeradd (a, b, a) - so use an intermediate to play safe */
void timerinc (struct timeval *total, struct timeval *incr)
{
    struct timeval result;

    timeradd (total, incr, &result);
    *total = result;
}

void timerdiv (struct timeval *total, int n, struct timeval *avg)
{

    double f = total->tv_sec;

    f += total->tv_usec / 1000000.0;

    if (n != 0)
	f /= n;

    avg->tv_sec = f;
    avg->tv_usec = (f - avg->tv_sec) * 1000000;
}


void new_pipe (db_thread *t, int *pipefd)
{

    if (pipe (pipefd) == -1)
    {
	perror ("pipe");
	exit (1);
    }
    if (t->globals->verbose > 1)
	printf ("%d: pipefd[0]=%d, pipefd[1]=%d\n",
		t->tid, pipefd[0], pipefd[1]);

}

/* As per man page for readdir_r() */
struct dirent *dirent_alloc (char *dirpath)
{
    int len, name_max;
    
    if ((name_max = pathconf (dirpath, _PC_NAME_MAX)) == -1)
	name_max = 255;
    len = offsetof (struct dirent, d_name) + name_max + 1;

    return (struct dirent *) malloc (len);
}

int check_for_dir (char *root, char *dirname)
{
    int            rv = 1;
    struct dirent *ent = dirent_alloc (root), *res;
    DIR           *dir = opendir (root);
    
    if (ent == NULL || dir == NULL)
    {
	perror (root);
	goto end;
    }
    if (dirname)
    {
	while (readdir_r (dir, ent, &res) == 0 && res != NULL)
	{
	    if (strcmp (dirname, ent->d_name) == 0)
	    {
#ifdef _DIRENT_HAVE_D_TYPE
		if (ent->d_type == DT_DIR)
#endif
		    rv = 0;
		break;
	    }
	}
    }
    else
	rv = 0;

end:

    if (dir)
	closedir (dir);
    if (ent)
	free (ent);
    
    return rv;
}

int next_file_in_dir (char *dirname, char *match, char *filename)
{
    int            rv = 1;
    struct dirent *ent = dirent_alloc (dirname), *res;
    DIR           *dir = opendir (dirname);
    
    if (ent == NULL || dir == NULL)
    {
	perror (dirname);
	goto end;
    }
    while (readdir_r (dir, ent, &res) == 0 && res != NULL)
    {
	if (strcmp (ent->d_name, "STOP") == 0)
	{
	    rv = -1;
	    goto end;
	}
	
	if (fnmatch (match, ent->d_name, 0) == 0)
	{
#ifdef _DIRENT_HAVE_D_TYPE
	    if (ent->d_type == DT_REG)
#else
	    if (1)
#endif
	    {
		rv = 0;
		strcpy (filename, ent->d_name);
	    }
	    break;
	}
    }

end:

    if (dir)
	closedir (dir);
    if (ent)
	free (ent);
    
    return rv;
}

int numeric_arg (int argc, char *argv[], int def)
{
    int value = -1;

    if (argc > 2)
	value = strtol (argv[2], NULL, 10);
    
    if (value < 0)
	value = def;

    return value;
}

int main (int argc, char *argv[])
{
    char     *prog = basename (argv[0]);
    char     *informixserver;
    db_thread t_main, *m = &t_main;
    node      n_main, *n = &n_main;
    global    g_main, *g = &g_main;
    int       i;

    /* Use a block of thread pointers - not very dynamic but can
       be grown when required */
    tlist = (db_thread **) calloc (BLOCK_SIZE, sizeof (db_thread *));
    thread_block_size = BLOCK_SIZE;
    
    memset (g, 0, sizeof (global));
    memset (n, 0, sizeof (node));
    memset (m, 0, sizeof (db_thread));

    /* First thread is static main */
    m->thread = pthread_self ();
    m->tid = 0;
    m->globals = g;
    m->main = m;
    m->type = T_MAIN;
    m->name = "Init";
    tlist[0] = m;
    thread_total++;
    
    
    /* Set up initial defaults */
    g->txsize = -1;
    g->put_buffer = BUF_SIZE;
    g->put_count = -1;
    g->dtformat = DEFAULT_DTFORMAT;
    g->ifs = '|';
    g->nthreads = 0;
    g->in_threads = 1;
    g->input = NULL;
    g->collect_info = 1;
    g->server_list = n;
    g->flushflag = 1;
    g->flush_interval_tv.tv_sec = 0;
    g->flush_interval_tv.tv_usec = 0;
    g->meta_nodeid = "nodeid";
    g->unique_id = 0;
    g->skip_cols = -1;
    g->pass_through = -1;
    g->one_shot = 0;
    g->repeat_marker = DEFAULT_REPEAT_MARKER;

    m->unique_id = g->unique_id++;

    /* Collect arguments */
    while (argc > 1 && *argv[1] == '-')
    {
	if (strcmp (argv[1], "--help") == 0)
	    usage (prog);
	else if (strcmp (argv[1], "--options") == 0)
	{
	    options ();
	    exit (0);
	}
	else if (strcmp (argv[1], "--version") == 0)
	    version ();
	else if (strcmp (argv[1], "--man") == 0)
	    man ();
	else if (strcmp (argv[1], "--verbose") == 0)
	    g->verbose++;
	else if (strcmp (argv[1], "--reduced-log") == 0)
	    g->reduced_log = 1;
	else if (strcmp (argv[1], "--remove-allnulls") == 0)
	    g->remove_all_nulls = 1;
	else if (strcmp (argv[1], "--no-info") == 0)
	    g->collect_info = 0;
	else if (strcmp (argv[1], "--fix-non-ascii") == 0)
	    g->fix_non_ascii = 1;
	else if (strcmp (argv[1], "--mod-id-queue") == 0)
	    m->mod_queue = 1;
	else if (strcmp (argv[1], "--one-shot") == 0)
	    g->one_shot = 1;
	else if (strcmp (argv[1], "--isolation") == 0)
	{
	    g->isolation = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--threads") == 0)
	{
	    g->set_nthreads = 1;
	    g->nthreads = numeric_arg (argc, argv, 1);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--readers") == 0)
	{
	    g->in_threads = numeric_arg (argc, argv, 1);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--status") == 0)
	{
	    g->status_update = numeric_arg (argc, argv, 0);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--lock-containers") == 0)
	    g->lock_containers = 1;
	else if (strcmp (argv[1], "--to-utc") == 0)
	    g->to_utc = 1;
	else if (strcmp (argv[1], "--flush-flag") == 0)
	{
	    g->flushflag = numeric_arg (argc, argv, 1);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--flush-interval") == 0)
	{
	    g->flush_interval_tv.tv_sec = numeric_arg (argc, argv, DEFAULT_FLUSH_INTERVAL);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--skip-repeat-columns") == 0)
	{
	    g->skip_cols = numeric_arg (argc, argv, 0);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--pass-through") == 0)
	{
	    g->pass_through = numeric_arg (argc, argv, 0);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--repeat-marker") == 0)
	{
	    g->repeat_marker = numeric_arg (argc, argv, DEFAULT_REPEAT_MARKER);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--tx-size") == 0)
	{
	    g->txsize = numeric_arg (argc, argv, DEFAULT_TXSIZE);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--lock-wait") == 0)
	{
	    g->lock_wait = numeric_arg (argc, argv, DEFAULT_LOCKWAIT);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--put-buffer") == 0)
	{
	    g->put_buffer = numeric_arg (argc, argv, BUF_SIZE);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--put-count") == 0)
	{
	    g->put_count = numeric_arg (argc, argv, 1);
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--datetime") == 0)
	{
	    g->dtformat = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--format") == 0)
	{
	    g->inputformat = argv[2];
	    argc--; argv++;
	}
#ifdef TODO_new_ts
	else if (strcmp (argv[1], "--new-ts") == 0)
	{
	    g->new_ts = argv[2];
	    argc--; argv++;
	}
#endif
	else if (strcmp (argv[1], "--database") == 0)
	{
	    g->database = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--user") == 0)
	{
	    g->user = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--password") == 0)
	{
	    g->password = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--table") == 0)
	{
	    g->table = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--column") == 0)
	{
	    g->column = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--input-dir") == 0)
	{
	    g->input_dir = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--putbuf-dir") == 0)
	{
	    g->putbuf_dir = argv[2];
	    argc--; argv++;
	}
#ifdef ENABLE_GRID
	else if (strcmp (argv[1], "--meta-table") == 0)
	{
	    g->meta_table = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--meta-join") == 0)
	{
	    g->meta_join = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--meta-nodeid") == 0)
	{
	    g->meta_nodeid = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--grid") == 0)
	{
	    g->grid = argv[2];
	    argc--; argv++;
	}
#endif
	else if (strcmp (argv[1], "--servers") == 0)
	{
	    g->servers = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--rejects") == 0)
	{
	    g->rejects = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--tsl-rejects") == 0)
	{
	    g->tsl_rejects = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--logfile") == 0)
	{
	    g->logfile = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--where") == 0)
	{
	    g->where = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--input") == 0)
	{
	    g->input = argv[2];
	    argc--; argv++;
	}
	else if (strcmp (argv[1], "--ifs") == 0)
	{
	    g->ifs = argv[2][0];
	    argc--; argv++;
	}
	else
	{
	    printf ("Unknown flag <%s>\n", argv[1]);
	    usage (prog);
	    exit (1);	    
	}
	argc--; argv++;
    }

    /* Validate arguments */
    if (g->database == NULL)
    {
	printf ("Must specify a database name\n");
	usage (prog);
	exit (1);
    }
    if (g->table == NULL || g->column == NULL)
    {
	printf ("Must specify table and column\n");
	usage (prog);
	exit (1);
    }
    if (g->grid) 
    {
	int err = 0;
	if (g->meta_table == NULL)
	{
	    printf ("Cannot load directly into a TimeSeries GRID. You must specify a sharded metadata table\n");
	    err = 1;
	}
	if (g->servers)
	{
	    printf ("You can only specify --grid or --servers, not both\n");
	    err = 1;
	}
	if (err)
	    exit (1);
    }

    /* Need to set this before any database activity */
    if ((informixserver = getenv ("INFORMIXSERVER")) == NULL)
    {
	printf ("Must set INFORMIXSERVER environment\n");
	exit (1);
    }

    n->name = strdup (informixserver);
    n->id = 0;
    
    if (g->grid == NULL && g->servers == NULL)
	g->servers = informixserver;
    
    if (getenv ("FET_BUF_SIZE") == NULL)
    {
	char buf[32];
	/* g->put_buffer may get resized later but that shouldn't matter */
	snprintf (buf, sizeof (buf), "FET_BUF_SIZE=%d", g->put_buffer * 2);
	putenv (buf);
    }
	
    if (db_connect (m, 0, T_MAIN) == 0)
    {
	printf ("Could not connect Main thread to database <%s@%s>\n",
		m->globals->database,
		m->globals->server_list[0].name);
	exit (1);
    }

    verbose = g->verbose;
    
    g->nservers = get_cluster_info (m);

    /* This will set the number of threads for each server to the
       number of CPUs available. */
    if (g->nthreads < 2)
    {
	db_get_cpu_info (m);
	g->nthreads = 1;
    }

    if (strcmp (prog, "TSL_Load") == 0)
    {
	if (g->rejects)
	{
	    FILE *rfile;
	    if ((rfile = fopen (g->rejects, "a")) == NULL)
	    {
		printf ("Could not open rejects file <%s>\n", g->rejects);
		exit (1);
	    }
	    fclose (rfile);
	}
	if (g->input == NULL && g->input_dir == NULL)
	{
	    printf ("Must specify input location as file or directory\n");
	    exit (1);
	}
	if (g->input != NULL && g->input_dir != NULL)
	{
	    printf ("You can only specify one type of input\n");
	    exit (1);
	}
	if (g->putbuf_dir)
	{
	    if (check_for_dir (g->putbuf_dir, NULL) != 0)
	    {
		printf ("Could not access directory for PutBuffer\n");
		perror (g->putbuf_dir);
		exit (1);
	    }
	}
	else
	    if (g->put_buffer > BUF_SIZE)
	    {
		g->put_buffer = BUF_SIZE;
		printf ("Warning: put-buffer can only be > BUF_SIZE when using --putbuf-dir, resetting to %d\n", g->put_buffer);
	    }
	if (g->input_dir)
	{
	    int errs = 0;
	    
	    errs += check_for_dir (g->input_dir, INPUT_DIR);
	    errs += check_for_dir (g->input_dir, PROCESS_DIR);
	    errs += check_for_dir (g->input_dir, COMPLETE_DIR);
	    
	    if (errs > 0)
	    {
		printf ("Input directory '%s' does not contain correct subdirectory structure\n", g->input_dir);
		if (g->verbose)
		    printf ("Required directories are:\n\t'%s/%s'\n\t'%s/%s'\n\t'%s/%s'\n",
			    g->input_dir, INPUT_DIR,
			    g->input_dir, PROCESS_DIR,
			    g->input_dir, COMPLETE_DIR);
		exit (1);
	    }
	    g->dir_readerfd = (int *) calloc (g->in_threads, sizeof (int));
	}
 	return TSL_Load (m);
    }
    else if (strcmp (prog, "TSL_Shutdown") == 0)
    {
	for (i = 0; i < g->nservers; i++)
	{
	    n = &(g->server_list[i]);
	    n->handle = malloc (strlen(g->table) + strlen(g->column) + 2);
	    sprintf (n->handle, "%s|%s", g->table, g->column);
	}
	g->nthreads = 1;
	return TSL_ShutDown (m);
    }
    else if (strcmp (prog, "TSL_Init") == 0)
    {
	if (g->meta_table && g->meta_join == NULL)
	{
	    printf ("You must specify a join condition using '--meta-join' when using a meta table\n");
	    exit (1);
	}
	if (g->grid && g->set_nthreads == 0)
	{
	    printf ("%s: Using parallel threads to initialise %d GRID servers\n", m->name, g->nservers);
	    g->nthreads = g->nservers;
	}
	else if (g->set_nthreads == 0)
	    g->nthreads = 1;
	return TSL_Init (m);
    }
    else
    {
	printf ("No internal processing for <%s>\n", prog);
	usage (prog);
    }

    exit (0);
}


int start_thread (db_thread *t, char *name, void *(*pfunc)(void *))
{
    int pipefd[2];

    t->name = name;
    t->pfunc = pfunc;

    new_pipe (t, pipefd);
    t->pipe = pipefd[0];

    if (pthread_create (&(t->thread), NULL, t->pfunc, t) != 0)
    {
	char msg[128];

	sprintf (msg, "Could not create thread: %s\n", t->name);
	perror (msg);
	exit (1);
    }

    /* One of these should work - except on AIX ;-( */
#ifdef HAVE_pthread_setname_np
    pthread_setname_np (t->thread, name);
#endif

#ifdef PR_SET_NAME
    prctl (PR_SET_NAME, name, 0, 0, 0);
#endif

    return pipefd[1];
}

long collect_thread (db_thread *t)
{
    long    retval;
    void   *v_ptr = (void *)&retval;

    pthread_join (t->thread, &v_ptr);
    
    if (t->connection)
	free_connection (t->connection);
    
    t->connection = NULL;
    
    return retval;
}


void destroy_thread (db_thread *t)
{
    int id;
    
    /* Find this thread in the main list */
    for (id = 1; id < thread_total; id++)
        if (tlist[id] == t)
            break;
    
    if (id == thread_total)
        printf ("ODD: %s[%d] Thread not in global list\n", t->name, id);
    else
        tlist[id] = NULL;


    if (t->connection)
	printf ("ODD: %s[%d] Thread still has a valid connection\n",
		t->name, t->tid);
    
    if (t->name)
	free (t->name);
    
    free (t);

    thread_total--;
    
}

int on_all_servers (db_thread *m, char *uname, void *(*func)(void *),
		    t_type type)
{
    db_thread **ilist = NULL;
    global     *g;
    char        tname[IDENT_LEN+32];
    int         node, rv;

    g = m->globals;

    rv = 0;
    for (node = 0; node < g->nservers; node++, g->unique_id++)
    {
	if (g->nthreads == 0)
	{
	    db_connect (m, node, type);
	    rv += *(long *) func (m);
	}
	else
	{
	    if (ilist == NULL)
		ilist = (db_thread **) calloc (g->nservers, sizeof (db_thread *));
	    sprintf (tname, "%s-%d", uname, node);
	    ilist[node] = init_thread (m, type);
	    ilist[node]->connection = create_connection (node, type, ilist[node]->tid);
	    start_thread (ilist[node], strdup (tname), func);
	}
    }

    if (g->nthreads == 0)
    {
	if (m->connection)
	    db_disconnect (m);
    }
    else if (ilist != NULL)
    {		
	for (node = 0; node < g->nservers; node++)
	{
	    if (ilist[node] != NULL)
	    {
		rv += collect_thread (ilist[node]);
		destroy_thread (ilist[node]);
	    }
	}
    }

    if (ilist)
	free (ilist);

    return rv;
    
}

int TSL_Init (db_thread *m)
{
    on_all_servers (m, "Init", init, T_MAIN);

    return 0;
}

int TSL_ShutDown (db_thread *t)
{
    int node;

    t->name = strdup ("Shutdown");

    db_disconnect (t);
    
    /* No point doing this in parallel - it is a quick operation */
    for (node = 0; node < t->globals->nservers; node++)
    {
	if (db_connect (t, node, T_MAIN) == 0)
	{
	    printf ("Could not connect to database <%s@%s> for shutdown\n",
		    t->globals->database,
		    t->globals->server_list[node].name);
	    exit (1);
	}
    
	tsl_shutdown (t);

	db_disconnect (t);
    }
    
    return 0;
}

int get_cluster_info (db_thread *m)
{
    char *old_name = m->name;
    int n;

    m->name = strdup ("Cluster");
    
    /* As a side effect this will count the rows in the tables 
       and also get the remote systems CPU VP info. */
    if (m->globals->grid)
	n = db_grid_list (m);
    else
	n = db_server_list (m);
    
    free (m->name);
	
    m->name = old_name;

    return n;
}

int set_queues (db_thread *m)
{
    int     i, r = 0, server;
    node   *n;
    global *g;

    g = m->globals;
    
    for (server = 0; server < g->nservers; server++)
    {
	n = &(g->server_list[server]);
    
	for (r = i = 0; i < n->container_hash_total; i++)
	{
	    n->container_list[i]->queue = r++ % n->nqueues;
	    if (g->verbose > 1)
		printf ("%s-%s: Put container <%s> on queue %d\n",
			m->name, n->name, n->container_list[i]->name,
			n->container_list[i]->queue);
	}
    
    }

    return r;
}

char *t_name (int *id)
{
    pthread_t p = pthread_self();
    int       i;

    if (id)
	*id = -1;

    /* If we weren't concerned about our thread id then
       pthread_getname_np() would be an alternative */
    
    if (tlist)
	for (i = 0; i < thread_total; i++)
	    if (pthread_equal (tlist[i]->thread, p) && tlist[i]->name)
	    {
		if (id)
		    *id = tlist[i]->tid;
		return tlist[i]->name;
	    }

    return "Unknown";
}

void set_formatting (db_thread *m)
{
    char   **keys, **values;
    int      i;
    global  *g;

    g = m->globals;
    
    if (g->inputformat == NULL)
	return;

    if (strncmp (g->inputformat, "JSON", 4) == 0 ||
	strncmp (g->inputformat, "json", 4) == 0)
    {
	// Parse the JSON document to get the column positions
	g->json_ncols = json_parse_string_pairs (g->inputformat+4, &keys, &values);
	g->json_key_names = calloc (g->json_ncols, sizeof (char *));

	if (g->verbose)
	    printf ("%s: JSON document with %d columns\n", m->name,
		    g->json_ncols); 

	for (i = 0; i < g->json_ncols; i++)
	{
	    int v = strtol (values[i], NULL, 10);
	    if (v < 0 || v >= g->json_ncols)
	    {
		printf ("Cannot parse format value %d\n", v);
		exit (1);
	    }
	    g->json_key_names[v] = keys[i]; 
	}
    }
}

int get_hash_lists (db_thread *m)
{
    on_all_servers (m, "Hash", hashlists, T_MAIN);

    return 0;
}

/* This should only be called from the main context */
db_thread *init_thread (db_thread *m, t_type type)
{
    int        id;
    db_thread *t = (db_thread *) calloc (1, sizeof (db_thread));
    
    /* Find the next empty slot in the main list */
    for (id = 1; id < thread_block_size; id++)
    {
	if (tlist[id] == NULL)
	    break;
    }
    /* Need to grow list */
    if (id == thread_block_size)
    {
	db_thread **new_tlist;

	if (m != m->main)
	{
	    printf ("%s Error - cannot re-allocate thread list outside of main context\n", m->name);
	    exit (1);
	}
	if (m->globals->verbose > 0)
	    printf ("%s: Allocating a new block of %d threads\n",
		    m->name, BLOCK_SIZE);
	
	new_tlist = (db_thread **) calloc (thread_block_size + BLOCK_SIZE, sizeof (db_thread *));
	memcpy (new_tlist, tlist, sizeof (db_thread *) * (thread_total));
	thread_block_size += BLOCK_SIZE;
	free (tlist);
	tlist = new_tlist;
    }
    tlist[id] = t;
    t->globals = m->globals;
    t->tid = id;
    t->type = type;
    thread_total++;

    return t;
}

void sig_handler (int sig)
{

    if (sig == SIGINT)
    {
	if (verbose)
	    printf ("Caught end process signal\n");
	end_process = 1;
    }
    else if (verbose)
	printf ("Caught signal %d\n", sig);
}


int TSL_Load (db_thread *m)
{
    time_t     now, start;
    int        rv = 0, i, server;
    global    *g;
    node      *n;
    db_thread *tstatus = NULL;
    struct timeval   avg_tv;
    struct sigaction act;

    g = m->globals;
    
    act.sa_handler = sig_handler;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    
    sigaction (SIGUSR1, &act, NULL);
    sigaction (SIGINT, &act, NULL);
    
    /* Check argument compatibility and set defaults */
    if (g->set_nthreads == 1 && g->nthreads == 0)
    {
	int   bad = 0;
	
	if (g->to_utc)
	{
	    printf ("Warning - Cannot use --to-utc in single threaded mode\n");
	    bad = 1;
	}
	if (g->lock_containers)
	{
	    printf ("Warning - Cannot use --lock-containers in single threaded mode\n");
	    bad = 1;
	}
	if (bad)
	{
	    printf ("Using nthreads=1\n");
	    g->nthreads = 1;
	}
    }

    if (g->set_nthreads == 0 && g->nthreads == 0)
    {
	g->nthreads = g->nservers;
    }
    
    if (g->txsize != -1 && g->lock_containers && g->put_count > 1)
	printf ("Warning - because container locking is specified the tx-size will override the put-count setting\n");

    if (g->txsize != -1 && g->lock_containers && g->put_count == -1)
	g->txsize = DEFAULT_TXSIZE;

    if (g->put_count == -1)
	g->put_count = 1;
    
    set_formatting (m);

    get_pk_info (m);

    g->total_nrows = get_total_nrows (m);

    db_disconnect (m);

    get_hash_lists (m);

    if (g->nthreads > 0)
    {
	int     nrows, ninsert;
	time_t  start;
	
	hash_create (&g->pk_hash, g->total_nrows);

	start = time (NULL);
	
	for (ninsert = nrows = server = 0; server < g->nservers; server++)
	{
	    n = &(g->server_list[server]);
	    if (n->nqueues == 0)
		n->nqueues = g->nthreads;
	    for (i = 0; i < n->nrows; i++, nrows++)
	    {
		ts_data *d = n->ts_data_list[i];

		if (hash_search (&g->pk_hash, d->key) != NULL)
		{
		    if (g->verbose > 1)
			printf ("%s: Found duplicate of <%s>\n",
				m->name, d->key);
		    continue;
		}
		else if (hash_insert (&g->pk_hash, d->key, d) == NULL)
		{
		    if (g->verbose > 1)
			printf ("%s: Error on hash insert of <%s>\n",
				m->name, d->key);
		    continue;
		}
		ninsert++;
	    }
	}
	if (g->verbose)
	{
	    printf ("%s: Inserted %d rows into global hash map - %ld seconds\n",
		    m->name, ninsert, time (NULL) - start);
	    HSTATS (&g->pk_hash);
	}

	set_queues (m);
    }
    
    
    /* We make a distiction here between 0 loader threads, where the
       main thread does all the work, and 1 loader thread. For the true
       single threaded case there is no need to create the hash maps.
       For the threads = 1 case we build the hash maps but they are
       only really useful for debugging or if there is a need to lock
       containers. Practically zero use in a clustered environment
       where the hashlist must be maintained for the node mapping.

       TODO: Remove this feature. It doesn't make a whole lot of sense
       when one prime reason for a high speed loader would be to make
       the best uses of a multi-processor system.
    */

    if (g->nthreads == 0)
    {
	/* Optimise for a single thread */
	m->tid = 0;
	m->unique_id = m->globals->unique_id++;
	printf ("Single loader process\n");
	time (&start);
    
	rv = *(int *) loader (m);
    }
    else
    {
	db_thread *t, *file_reader = NULL;
	char       tname[IDENT_LEN+32];

	g->dir_reader = (db_thread **) calloc (g->in_threads, sizeof (db_thread *));
	printf ("Starting status thread\n");
	t = init_thread (m, T_STATUS);
	g->statusfd = start_thread (t, strdup ("Status"), status);

	for (server = 0; server < g->nservers; server++)
	{
	    int per_thread;
	    n = &(g->server_list[server]);

	    printf ("Starting provisioning thread on server %d\n", server);
	    sprintf (tname, "Provision-%d", server);
	    n->provision_thread = init_thread (m, T_PROVISION);
	    n->provision_thread->connection = create_connection (server, T_PROVISION, n->provision_thread->tid);
	    n->provisionfd = start_thread (n->provision_thread, strdup (tname), provision);

	    per_thread = (n->container_hash_total / n->nqueues) + 1;
	    
	    printf ("Starting %d loader threads on node <%s>, each managing %d containers\n",
		    n->nqueues, n->name, per_thread);
	    n->pipelist = (int *) calloc (n->nqueues, sizeof (int));
	    n->threadlist = (db_thread **) calloc (n->nqueues, sizeof (db_thread *));
	    for (i = 0; i < n->nqueues; i++)
	    {
		int j;
		n->threadlist[i] = t = init_thread (m, T_LOADER);
		sprintf (tname, "Loader-%d@%d", i, server);
		/* Build the list of containers that this thread manages */
		t->ncontainers = 0;
		t->container_list = (container **) calloc (per_thread,
							   sizeof (container *));
		/* Keep a separate buffer for each container so that
		   the flush will be optimally opening/closing the
		   container partition */
		for (j = 0; j < n->container_hash_total; j++)
		{
		    if (n->container_list[j]->queue == i)
		    {
			if (g->verbose > 1)
			    printf ("%s; Assigned container <%s> to thread %d\n",
				    t->name, n->container_list[j]->name, i);
			t->container_list[t->ncontainers++] = n->container_list[j];
		    }
		}
		t->connection = create_connection (server, T_LOADER, t->tid);
		n->pipelist[i] = start_thread (t, strdup (tname), loader);
	    }
	    if (n->dev_nrows > n->nrows)
	    {
		printf ("%s: Starting storage thread for node %s\n",
			m->name, n->name);

		sprintf (tname, "TS-Store@%d", server);

		t = init_thread (m, T_STORAGE);
		t->connection = create_connection (server, T_STORAGE, t->tid);

		n->storagefd = start_thread (t, strdup (tname), storage);

		printf ("%s: Starting re-write thread for node %s\n",
			m->name, n->name);

		sprintf (tname, "Re-Write@%d", server);

		t = init_thread (m, T_REWRITE);
		t->connection = create_connection (server, T_REWRITE, t->tid);

		n->rewritefd = start_thread (t, strdup (tname), rewrite);
	    }
	}

	time (&start);	

	end_process = 0;
	
	if (g->input)
	{
	    printf ("Starting input reader thread\n");
	    file_reader = t = init_thread (m, T_READER);
	    t->connection = create_connection (0, T_READER, t->tid);    
	    g->readerfd = start_thread (t, strdup ("Reader"), reader);
	}

	if (g->input_dir)
	{
	    if (g->in_threads == 1)
		printf ("Starting directory processing thread\n");
	    else
		printf ("Starting %d directory processing threads\n",
			g->in_threads);
	    for (i = 0; i < g->in_threads; i++)
	    {
		char rname[32];
		sprintf (rname, "DirReader-%d", i);
		g->dir_reader[i] = t = init_thread (m, T_DIR_READER);
		t->connection = create_connection (0, T_READER, t->tid);    
		g->dir_readerfd[i] = start_thread (t, strdup (rname), reader);
	    }
	}

	for ( ; end_process == 0; )
	{
	    int dir_end = 0;
	    if (g->dir_reader[0])
	    {
		for (dir_end = g->in_threads, i = 0; i < g->in_threads; i++)
		{
		    if (g->dir_reader[i] && g->dir_reader[i]->treturn != 0)
			dir_end--;
		}
	    }
	    /* Only one or the other is running */
	    if ((file_reader && file_reader->treturn != 0) ||
		(g->dir_reader[0] && dir_end == 0))
		end_process = 1;

	    sleepfor (1);

	    if (g->flush_interval_tv.tv_sec > 0)
	    {
		int    queue;
		struct timeval now_tv, overdue_tv;
		int    flush_candidates;

		gettimeofday (&now_tv, NULL);
		timersub (&now_tv, &(g->flush_interval_tv), &overdue_tv);
		
		for (server = 0; server < g->nservers; server++)
		{
		    n = &(g->server_list[server]);
		    for (queue = 0; queue < n->nqueues; queue++)
		    {
			db_thread *l = n->threadlist[queue];

			/* Have done puts but no flush in the last interval */
			if (l->puts_this_flush > 0 &&
			    timercmp (&(l->last_flush_tv), &overdue_tv, <))
			{
			    if (g->verbose > 1)
				printf ("Need to flush node %d <%s>, queue %d <%s>\n",
					server, n->name, queue, l->name);
			    flush_candidates = 1;
			}
			else
			{
			    /* Check for partially full buffers */
			    flush_candidates = 0;
			    if (timercmp (&(t->last_put_tv), &overdue_tv, <) &&
				    *t->buffer)
			    {
				if (g->verbose > 1)
				    printf ("Need to flush node %d <%s>, queue %d <%s>\n",
					    server, n->name, queue,
					    l->name);
				flush_candidates++;
			    }
			}
			if (flush_candidates)
			    send_flush_thread (l, n, queue);
		    }
		}
	    }
	}

	shutdown_all (m);

	time (&now);
	
	/* Wait for all threads to finish */
	for (i = 1; i < thread_total; i++)
	{
	    if (tlist[i])
		collect_thread (tlist[i]);
	}

	/* Collect summary stats */
	for (i = 0; i < thread_total; i++)
	{
	    if (tlist[i])
	    {
		t = tlist[i];
	    
		if (t->type == T_LOADER && t->treturn == 0)
		{
		    /* Stats from flush */
		    m->errors     += t->errors;
		    m->elements   += t->elements;
		    m->duplicates += t->duplicates;
		    m->commits    += t->commits;
		    m->rollbacks  += t->rollbacks;
		    m->exceptions += t->exceptions;
		    
		    /* Internal counters */
		    m->nint       += t->nint;
		    m->nput       += (t->nput > 0 ? t->nput : 0); 
		    m->nflush     += t->nflush;
		    m->nallnulls  += t->nallnulls;
		    
		    /* Timers */
		    timerinc (&(m->total_flush_elapsed), &(t->total_flush_elapsed));
		    timerinc (&(m->total_put_elapsed), &(t->total_put_elapsed));
		}
		if (t->type == T_READER)
		    m->rejects += t->rejects;
		if (t->type == T_STATUS)
		    tstatus = t;
	    }
	}
    }

    now -= start;

    printf ("\nProcessed %d incoming records\n", m->nint);
    if (g->collect_info)
    {
	printf ("Extended stats:\n\tduplicates\t= %d\n\tcommits   \t= %d\n\trollbacks\t= %d\n\texceptions\t= %d\n\terrors   \t= %d\n",
		m->duplicates,
		m->commits,
		m->rollbacks,
		m->exceptions,
		m->errors);
    }
    else if (m->errors > 0)
	printf ("Saw %d error%s\n", m->errors, (m->errors == 1 ? "" : "s"));
    
    if (g->collect_info)
    {
	printf ("Inserted %d element%s", m->elements,
		(m->elements == 1 ? "" : "s"));
	if (m->rejects > 0)
	    printf (" - %d %s by reader\n", m->rejects,
		    g->fix_non_ascii ? "fixed" : "rejected");
	else
	    printf ("\n");
	if (m->nallnulls > 0)
	    printf ("Removed %d all NULL rows\n", m->nallnulls); 
    }
    else
	m->elements = m->nint;
    
    printf ("Used %d put%s and %d flush%s\nTook %ld seconds - %d per second\n",
	    m->nput, (m->nput == 1 ? "" : "s"),
	    m->nflush, (m->nflush == 1 ? "" : "es"),
	    now, m->elements/(int)(now == 0 ? 1L : now));

    printf ("\tTime in TSL_Put %ld.%06d",
	    m->total_put_elapsed.tv_sec, (int) m->total_put_elapsed.tv_usec);
    timerdiv (&(m->total_put_elapsed), g->nthreads, &avg_tv);
    if (tstatus)
	printf (" (max %ld.%06d, per-thread %ld.%06d)",
		tstatus->put_elapsed.tv_sec, (int) tstatus->put_elapsed.tv_usec,
		avg_tv.tv_sec, (int) avg_tv.tv_usec);
    printf ("\n\tTime in TSL_Flush %ld.%06d",
	    m->total_flush_elapsed.tv_sec, (int) m->total_flush_elapsed.tv_usec);
    timerdiv (&(m->total_flush_elapsed), g->nthreads, &avg_tv);
    if (tstatus)
	printf (" (max %ld.%06d, per-thread %ld.%06d)",
		tstatus->flush_elapsed.tv_sec, (int) tstatus->flush_elapsed.tv_usec,
		avg_tv.tv_sec, (int) avg_tv.tv_usec);
    printf ("\n");

    exit (rv);
}
