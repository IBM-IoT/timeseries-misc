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
 *  Title            : database.ec
 *  Author           : Cosmo@uk.ibm.com
 *  Description      : SQL driver for Parallel TimeSeries Loader
 *
 *  Version          : 2.0	2014-03-06 Extensively re-written for 6.00.xC2
 *
 ****************************************************************************
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <libgen.h>

$define IDENT_LEN 128;
$define MAX_NODES 255;
/* This needs to be big enough to hold the longest concatenated primary key.
   Maximum ley length is based on page size. Minimum is 390 for a 2K page. */
$define COL_BUF_SIZE (390*4)+16;

$include "sql_defs.h";

#include "mi_search.h"

#include "defs.h"

#define TSOPEN_REDUCED_LOG              0x0100

#define TSL_LOG_NOLOG       0
#define TSL_LOG_OUTFILE     1
#define TSL_LOG_QUEUED      2
#define TSL_LOG_SYSTEMLOG   3

#define TSL_DEBUG          -1
#define TSL_OK              0
#define TSL_INFO            1
#define TSL_WARNING         2
#define TSL_ERROR           4

#define SQL_Check whenexp_chk_line(__LINE__),_dummy

$include sqlstype;
$include sqltypes;
$include sqlca;

int tsl_version = 0;

char *stmt_id (db_conn *c, char *id)
{
    char *new_id = (char *) malloc (strlen (id) + 16 + 1);

    sprintf (new_id, "_%d_%s", c->conn_id, id);

    return new_id;
}

/* Need two different functions because the $WHENEVER statement is a
   pre-compiler directive and is not evaluated at runtime */
int connect_with_error (db_thread *t, int node)
{
    global  *g = t->globals;
    $char  database[(IDENT_LEN*2)+2];
    $db_prep *p;
    $char  *passwd = g->password;
    $char  *user = g->user;

    p = t->connection->prepared;
    
    sprintf (database, "%s@%s", g->database,
	     g->server_list[node].name);    
    
    $WHENEVER ERROR CALL SQL_Check;

    if (user)
	$CONNECT TO $database AS $p->connection_id
	    USER $user USING $passwd;
    else
	$CONNECT TO $database AS $p->connection_id;

    return (sqlca.sqlcode == 0 ? 1 : 0);
}

int connect_without_error (db_thread *t, int node)
{
    global  *g = t->globals;
    $char  database[(IDENT_LEN*2)+2];
    $db_prep *p;
    $char  *passwd = g->password;
    $char  *user = g->user;

    p = t->connection->prepared;

    sprintf (database, "%s@%s", g->database,
	     g->server_list[node].name);    
    
    $WHENEVER ERROR CONTINUE;
    
    if (g->user)
	$CONNECT TO $database AS $p->connection_id
	    USER $user USING $passwd;
    else
	$CONNECT TO $database AS $p->connection_id;

    return (sqlca.sqlcode == 0 ? 1 : 0);
}

$WHENEVER ERROR CALL SQL_Check;

int connect_to (db_thread *t)
{
    int        connected = 0;
    db_conn   *c;
    global    *g;
    $char      wait_stmt[32];
    
    g = t->globals;
    c = t->connection;

    if (g->verbose)
	printf ("%s: id=%s connected=%d\n",
		t->name, c->prepared->connection_id, c->connected);

    if (g->verbose)
	connected = connect_with_error (t, c->current_server);
    else
	connected = connect_without_error (t, c->current_server);
    
    if (connected == 0)
	goto finished;
    
    $WHENEVER ERROR CALL SQL_Check;

    if (g->isolation)
    {
	if (strcmp (g->isolation, "RR") == 0)
	{
	    $SET ISOLATION REPEATABLE READ;
	}
	else if (strcmp (g->isolation, "CR") == 0)
	{
	    $SET ISOLATION COMMITTED READ;
	}
	else if (strcmp (g->isolation, "CS") == 0)
	{
	    $SET ISOLATION CURSOR STABILITY;
	}
	else if (strcmp (g->isolation, "DR") == 0)
	{
	    $SET ISOLATION DIRTY READ;
	}
	else 
	    printf ("%s: Unknown isolation level <%s>\n",
		    t->name, g->isolation);
    }

    if (g->lock_wait == 0)
	sprintf (wait_stmt, "SET LOCK MODE TO NOT WAIT");
    else
    {
	sprintf (wait_stmt, "SET LOCK MODE TO WAIT %d", g->lock_wait);
    
	if (g->lock_wait == -1)
	    wait_stmt[21] = '\0';
    }
    
    $EXECUTE IMMEDIATE $wait_stmt;
    
finished:

    return connected;
}

/* This sets up the data structures for the prepared statements
   an also associates the connection with a particular node */
db_conn *create_connection (int node, int type, int id)
{
    db_conn   *c = (db_conn *) calloc (1, sizeof (db_conn) + sizeof (db_prep));
    db_prep   *p;

    if (c == NULL)
	return NULL;

    c->prepared = p = (db_prep *) (c + 1);
    c->conn_id = id;
    c->conn_type = type;
    
    switch (type)
    {
    case T_MAIN:
	p->init_table_id         = stmt_id (c, "init_table");
	p->release_id            = stmt_id (c, "release");
	p->shutdown_id           = stmt_id (c, "shutdown");
	break;
    case T_LOADER:
	p->attach_id             = stmt_id (c, "attach");
	p->put_data_id           = stmt_id (c, "put_data");
	p->put_sql_id            = stmt_id (c, "put_sql");
	p->flush_data_id         = stmt_id (c, "flush_data");
	p->commit_data_id        = stmt_id (c, "commit_data");
	p->release_id            = stmt_id (c, "release");
	p->lock_container_id     = stmt_id (c, "lock_container");
	p->info_id               = stmt_id (c, "info_id");
	break;
    case T_STORAGE:
	p->storage_id            = stmt_id (c, "storage");
	break;
    case T_REWRITE:
	p->next_device_id        = stmt_id (c, "next_device");
	break;
    case T_READER:
    case T_PROVISION:
	break;
    default:
	printf ("ODD: No specific connection type %d\n", type);
	break;
    }

    p->connection_id         = stmt_id (c, "connection");

    c->current_server = node;

    c->connected = T_UNKNOWN;
    
    return c;
}
    
int db_connect (db_thread *t, int node, t_type type)
{
    struct  db_conn_s    *c;
    $struct db_prep_s    *p;
    $char                 version[IDENT_LEN] = { '\0' };

    if (t->connection == NULL)	
	t->connection = create_connection (node, type, t->tid);

    c = t->connection;
    p = c->prepared;
    
    if ((c->connected = connect_to (t)) == 0)
    {
	printf ("%s: Failed to connect to node %d\n",
		t->name, node);
	return 0;
    }	

    switch (type)
    {
    case T_MAIN:
	
	$EXECUTE FUNCTION TimeseriesRelease () INTO $version;
    
	if (t->globals->verbose)
	    printf ("%s: Server %s\n", t->name, version);
	
	$PREPARE $p->init_table_id
	    FROM "EXECUTE FUNCTION TSL_Init (?,?,?,?,?,?,?,?)";

	$PREPARE $p->release_id
	    FROM "EXECUTE FUNCTION TSL_SessionClose (?)";
	
	$PREPARE $p->shutdown_id
	    FROM "EXECUTE PROCEDURE TSL_Shutdown (?)";

	break;

    case T_LOADER:

        $EXECUTE PROCEDURE Ifx_Allow_Newline('t');
	
	$PREPARE $p->attach_id
	    FROM "EXECUTE FUNCTION TSL_Attach (?,?,?)";
	
	$PREPARE $p->put_data_id
	    FROM "EXECUTE FUNCTION TSL_Put (?,?)";
	
	$PREPARE $p->put_sql_id
	    FROM "EXECUTE FUNCTION TSL_PutSQL (?,?)";
	
	$PREPARE $p->release_id
	    FROM "EXECUTE FUNCTION TSL_SessionClose (?)";
	
	$PREPARE $p->lock_container_id
	    FROM "EXECUTE PROCEDURE TSContainerLock (?,?)";
	
	$PREPARE $p->flush_data_id
	    FROM "EXECUTE FUNCTION TSL_FlushAll (?,?)";
	
	$PREPARE $p->commit_data_id
	    FROM "EXECUTE FUNCTION TSL_Commit (?,?,?)";
    
	$PREPARE $p->info_id
	    FROM "SELECT info.* FROM TABLE (TSL_FlushInfo (?::LVARCHAR)) AS t (info)";

	break;

    case T_STORAGE:
    case T_REWRITE:

	$PREPARE $p->storage_id
	    FROM "INSERT INTO ts_storage VALUES (?, ?, ?)";

	$PREPARE $p->next_device_id
	    FROM "EXECUTE FUNCTION next_device ()";
	
	break;
	
    default:
	break;


    }
    
  return c->connected;
}

int table_attach (db_thread *t)
{
    $varchar    *s_tab_name, *s_ts_col,
 	         s_reject[IDENT_LEN*8] = "", s_handle[(IDENT_LEN*2)+1];
    $db_prep    *p = t->connection->prepared;
    $short       rejectind = 0;

    global   *g;
    int       rval;

    g = t->globals;

    s_tab_name = g->table;
    s_ts_col = g->column;

    if (g->verbose > 1)
	t->nput = -1;

    if (g->tsl_rejects == NULL)
	rejectind = -1;
    else
    {
	if (*g->tsl_rejects != '/')
	{
	    printf ("%s: Reject file must specify absolute path\n", t->name);
	    rval = -1;
	    goto end;
	}
	if (strstr (g->tsl_rejects, "%s"))
	    sprintf (s_reject, g->tsl_rejects, g->server_list[t->connection->current_server].name);
	else
	    strcpy (s_reject, g->tsl_rejects);
    }

    *s_handle = '\0';

    $WHENEVER ERROR CALL SQL_Check;
    
    $EXECUTE $p->attach_id INTO $s_handle USING
	$s_tab_name, $s_ts_col, $s_reject:rejectind;
    
    if (*s_handle == '\0')
    {
	printf ("%s: Could not locate TimeSeries loader segment for <%s.%s>\n",
		t->name, s_tab_name, s_ts_col);
	rval = -1;
    }
    else
    {
	g->server_list[t->connection->current_server].handle = strdup (s_handle);
	rval = 0;
    }
    
end:
    
    return rval;
}

int table_init (db_thread *t)
{
    $varchar *s_tab_name, *s_ts_col,
	      s_logfile[1024] = "", s_rejectfile[1024] = "",
	     *s_where, *s_dt_format, s_handle[(IDENT_LEN*2)+1];
    $int      i_logtype, i_loglevel;
    $db_prep *p = t->connection->prepared;
    $short    whereind = 0, logfileind = 0, rejectfileind = 0, dt_formatind = 0;

    global   *g;
    node     *n;
    int       rval;

    g = t->globals;
    n = &(g->server_list[t->connection->current_server]);    

    s_tab_name = g->table;
    s_ts_col = g->column;
    s_where = g->where;
    s_dt_format = g->dtformat;
    
    if (g->where == NULL)
	whereind = -1;

    if (g->logfile == NULL)
	logfileind = -1;

    if (g->tsl_rejects == NULL)
	rejectfileind = -1;

    else if ((g->logfile && *g->logfile != '/')
	     || (g->tsl_rejects && *g->tsl_rejects != '/'))
    {
	printf ("%s: Log files must specify absolute path\n", t->name);
	rval = -1;
	goto end;
    }
    if (g->dtformat == NULL)
	dt_formatind = -1;
    else if (*g->dtformat != '%')
    {
	printf ("%s: Date format should start with %%\n", t->name);
	rval = -1;
	goto end;
    }
    
    if (g->logfile)
    {
	if (strstr (g->logfile, "%s"))
	    sprintf (s_logfile, g->logfile, n->name);
	else
	    strcpy (s_logfile, g->logfile);
	i_logtype = TSL_LOG_OUTFILE;
    }		
    else
	i_logtype = TSL_LOG_SYSTEMLOG;
    i_loglevel = TSL_INFO;
    
    if (g->tsl_rejects)
    {
	if (strstr (g->tsl_rejects, "%s"))
	    sprintf (s_rejectfile, g->tsl_rejects, n->name);
	else
	    strcpy (s_rejectfile, g->tsl_rejects);
    }
    
    $WHENEVER ERROR CALL SQL_Check;
    
    $EXECUTE $p->init_table_id INTO $s_handle USING
	$s_tab_name, $s_ts_col, $i_logtype, $i_loglevel, $s_logfile:logfileind,
	$s_dt_format:dt_formatind, $s_rejectfile:rejectfileind,
	$s_where:whereind;
    
    if (*s_handle == '\0')
    {
	printf ("%s: Could not create loader segment for <%s.%s>\n",
		t->name, s_tab_name, s_ts_col);
	rval = -1;
    }
    else
    {
	n->handle = strdup (s_handle);
	rval = 0;
    }

end:

    return rval;
}

void db_get_cpu_info (db_thread *t)
{
    $char      sql_stmt[BUF_SIZE];
    global    *g = t->globals;
    node      *n;
    int        i;
    
    /* Get the number of cpus as a guide to how many queues to start */
    g->total_queues = 0;
    
    for (i = 0; i < g->nservers; i++)
    {
	$int ncpus;

	n = &(g->server_list[i]);
	
	sprintf (sql_stmt, "SELECT count(*) FROM sysmaster@%s:sysvpprof where class = 'cpu'", n->name);

	$PREPARE cpus_id FROM $sql_stmt;

	$EXECUTE cpus_id INTO $ncpus;

	$FREE cpus_id;

	if (g->verbose)
	    printf ("%s: Server %d:%s has %d CPU VPs, node info: devices=%d TimeSeries=%d\n", t->name, n->id, n->name, ncpus, n->dev_nrows, n->nrows);

	n->nqueues = ncpus;
	g->total_queues += ncpus;
    }

}

int db_server_list (db_thread *t)
{
    int     node_id;
    char   *sptr;
    global *g = t->globals;
    
    if (g->servers == NULL)
	return 0;

    /* First count number of servers */
    for (node_id = 1, sptr = g->servers; *sptr; sptr++)
    {
	if (*sptr == ',')
	    node_id++;
    }

    g->nservers = node_id;
    g->server_list = (node *) calloc (g->nservers, sizeof (node));

    g->server_list[0].id = 0;
    g->server_list[0].name = g->servers;

    for (node_id = 1, sptr = g->servers; *sptr; sptr++)
    {
	if (*sptr == ',')
	{
	    *sptr++ = '\0';
	    if (*sptr == '\0')
		break;
	    g->server_list[node_id].id = node_id;
	    g->server_list[node_id].name = sptr;
	    node_id++;
	}
    }

    for (node_id = 0; node_id < g->nservers; node_id++)
    {
	node  *n = &(g->server_list[node_id]);
	n->nrows = get_server_nrows (t, n);
	n->nqueues = g->nthreads;
    }
    
    return g->nservers;
}


int db_grid_list (db_thread *t)
{
    $varchar nodename[IDENT_LEN+1];
    $char    sql_stmt[BUF_SIZE];
    $int     nodeid;
    $int     nrows;
    node    *found_nodes = NULL;
    int      nodes, i, total_nrows;
    char    *tabname;
    global  *g;

    g = t->globals;
    
    /* Get server info first - nrows here is count of metadevice rows local to the node */
    found_nodes = (node *) calloc (MAX_NODES, sizeof (node));

    if (g->meta_table)
	tabname = g->meta_table;
    else
	tabname = g->table;
    
    sprintf (sql_stmt, "SELECT count(*), ifx_node_id(), TRIM(ifx_node_name()) FROM %s GRID ALL '%s' GROUP BY 2 ORDER BY 2",
	     tabname, g->grid);

    $PREPARE get_nodes_id FROM $sql_stmt;
    
    $DECLARE get_nodes CURSOR FOR get_nodes_id;

    $OPEN get_nodes;

    total_nrows = nodes = 0;
    while (sqlca.sqlcode == 0)
    {
	$FETCH get_nodes INTO $nrows, $nodeid, $nodename;
	
	if (sqlca.sqlcode == 0)
	{
	    found_nodes[nodes].name = strdup (nodename);
	    found_nodes[nodes].id = nodeid;
	    found_nodes[nodes].dev_nrows = nrows;
	    nodes++;
	    total_nrows += nrows;
	}
    }	
    $CLOSE get_nodes;

    $FREE get_nodes;
    $FREE get_nodes_id;

    if (nodes > 0)
    {
	g->nservers = nodes;
	g->server_list = (node *) calloc (nodes, sizeof (node));
	for (i = 0; i < nodes; i++)
	{
	    node *n = &(g->server_list[i]);
	    
	    n->id = found_nodes[i].id;
	    n->name = found_nodes[i].name;
	    n->dev_nrows = found_nodes[i].dev_nrows;
	    n->nqueues = g->nthreads;
	}
    }
    if (found_nodes)
	free (found_nodes);
    
    /* Get count of TimeSeries local to each node */
    sprintf (sql_stmt, "SELECT SUM(c), n FROM (SELECT count(*) c, %s n FROM %s GRID ALL '%s' GROUP BY 2) GROUP BY 2 ORDER BY 2;",
	     g->meta_nodeid,
	     g->meta_table, g->grid);
    
    $PREPARE get_nrows_id FROM $sql_stmt;
    
    $DECLARE get_nrows CURSOR FOR get_nrows_id;

    $OPEN get_nrows;

    while (sqlca.sqlcode == 0)
    {
	$FETCH get_nrows INTO $nrows, $nodeid;
	
	if (sqlca.sqlcode == 0)
	{
	    int f;

	    if ((f = find_server (t, nodeid)) != -1)
		g->server_list[f].nrows = nrows;
	}
    }
    
    $CLOSE get_nrows;

    $FREE get_nrows;
    $FREE get_nrows_id;

    g->total_nrows = total_nrows;

    return nodes;
}

int db_put_data (db_thread *t,
	      EXEC SQL BEGIN DECLARE SECTION;
	      parameter varchar *data
	      EXEC SQL END DECLARE SECTION;
    )
{
    $int      rval = -1;
    $varchar *s_handle = t->globals->server_list[t->connection->current_server].handle;
    $db_prep *p = t->connection->prepared;

    struct timeval start_tv;
    
    gettimeofday (&start_tv, NULL);
	
    $WHENEVER ERROR CALL SQL_Check;

    if (data && *data != '\0')
    {
	if (t->nput == -1 || t->globals->verbose > 1)
	{
	    printf ("%s: handle <%s>, data[%ld],                       \n%.256s\n",
		    t->name, s_handle, strlen (data), data);
	    t->nput = 0;
	}
	$EXECUTE $p->put_data_id INTO $rval USING $s_handle, $data;
    }

    gettimeofday (&(t->last_put_tv), NULL);
    
    timersub (&(t->last_put_tv), &start_tv, &(t->put_elapsed));
    timerinc (&(t->total_put_elapsed), &(t->put_elapsed));
    
    return rval;
}

int lock_container_sql (db_thread *t,
			EXEC SQL BEGIN DECLARE SECTION;
			parameter varchar *container,
			parameter int lock
			EXEC SQL END DECLARE SECTION;
    )
{
  $db_prep    *p = t->connection->prepared;
  int          i = 0;

  $WHENEVER ERROR CONTINUE;

  if (*container)
  {
      for (i = 0; i < 100; i++)
      {
	  $EXECUTE $p->lock_container_id USING $container, $lock;
	  if (sqlca.sqlcode == 0)
	      break;
      }
  }
  if (t->globals->verbose && i > 0)
      printf ("%s: Took %d attempts to lock container (sqlca.sqlcode=%d)\n",
	      t->name, i, sqlca.sqlcode); 

  $WHENEVER ERROR CALL SQL_Check;    

  return 0;
}

int lock_container (db_thread *t, char *container)
{
    return lock_container_sql (t, container, 1);
}

int unlock_container (db_thread *t, char *container)
{
    return lock_container_sql (t, container, 0);
}

container *new_container (char *name)
{
    int        len = sizeof (container) + strlen (name) + 1;
    container *new = (container *) calloc (1, len);
    
    if (new == NULL)
    {
	printf ("ERROR: No space for container + name\n");
	exit (1);
    }
    
    new->name = (char *)(new + 1);
    strcpy (new->name, name);
    
    return new;
}

container *store_container (char *cname, node *n)
{
    container *c;
    
    if (n->container_hash_total + 1 > n->container_table_size)
    {
	/* Bad ju-ju, container table is full */
	printf ("FATAL: Container table is full (%d)\n", n->container_table_size);
	exit (1);
    }
    
    if ((c = new_container (cname)) != NULL)
    {
	c->id = n->container_hash_total;
	hash_insert (&n->container_hash, c->name, c);
	n->container_list[n->container_hash_total] = c;
	n->container_hash_total++;
	gettimeofday (&(c->last_put_tv), NULL);
    }

    return c;
}

int get_containers (db_thread *t)
{
    $int     nrows;
    $varchar cname[IDENT_LEN+1];
    node    *n;
    global  *g;

    g = t->globals;
    n = &(g->server_list[t->connection->current_server]);
    
    if (n->container_list)
    {
	printf ("%s: WARNING: The container table is already allocated\n",
		t->name);
	return 0;
    }

    $WHENEVER ERROR CALL SQL_Check;
	
    $SELECT count(*) INTO $nrows FROM tscontainertable;

    /* Allow for growth */
    if (nrows == 0)
	n->container_table_size = MIN_CONTAINER_TABLE * 2;
    else
	n->container_table_size = nrows + MIN_CONTAINER_TABLE;
    
    n->container_list = (container **) malloc (nrows * sizeof (container *));
    n->container_hash_total = 0;

    hash_create (&n->container_hash, n->container_table_size);
	
    $DECLARE get_container CURSOR FOR
	SELECT TRIM(name) FROM tscontainertable;
	
    $OPEN get_container;
    
    while (sqlca.sqlcode == 0)
    {
	$FETCH get_container INTO $cname;
	
	if (sqlca.sqlcode == 0)
	    store_container (cname, n);
    }
    
    $CLOSE get_container;

    $FREE get_container;

    if (g->verbose)
	printf ("%s: Found %d containers\n", t->name, n->container_hash_total);

    return n->container_hash_total;
}

ts_data
*new_ts_data (char *key)
{
    int      len = sizeof (ts_data) + strlen (key) + 1;
    ts_data *new = (ts_data *) calloc (1, len);
    
    new->key = (char *)(new + 1);
    strcpy (new->key, key);

    return new;
}

int get_pk_info (db_thread *t)
{
    $varchar  all_cols [((IDENT_LEN+8)*32)+1];
    $int      col_len;
    $varchar *tabname;
    $varchar  col_pk_name[IDENT_LEN+1];
    $varchar  col_name[IDENT_LEN+1];

    global   *g;

    g = t->globals;
    
    if (g->meta_table)
	tabname = g->meta_table;
    else
	tabname = g->table;

    $WHENEVER ERROR CALL SQL_Check;
	
    $DECLARE get_pk_info CURSOR FOR
	SELECT 'TRIM('||t.tabname||'.'||TRIM(c.colname)||'::lvarchar)',
	TRIM(c.colname),
	schema_precision (c.coltype, c.extended_id, c.collength)
	FROM systables t, syscolumns c,
	(SELECT ikeyextractcolno(i.indexkeys, k.counter::INTEGER)
	 FROM systables t, sysconstraints p, sysindices i,
	 TABLE(MULTISET{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15})
	 AS k(counter)
	 WHERE t.tabname = $tabname AND t.tabid = p.tabid
	 AND t.tabid=i.tabid AND p.idxname = i.idxname AND p.constrtype = 'P')
	AS kp(colno)
	WHERE t.tabname = $tabname AND t.tabid=c.tabid AND c.colno = kp.colno
	ORDER BY c.colno;

    $OPEN get_pk_info;

    all_cols[0] = '\0';
    g->pk_column_count = 0;
    g->pk_column_len = 0;
    while (sqlca.sqlcode == 0)
    {
	$FETCH get_pk_info INTO $col_pk_name, $col_name, $col_len;
	
	if (sqlca.sqlcode == 0)
	{
	    g->pk_column_array[g->pk_column_count] = strdup (col_name);
	    g->pk_column_count++;
	    g->pk_column_len += (col_len+1);
	    if (all_cols[0] != '\0')
		strcat (all_cols, "||'|'||");
	    strcat (all_cols, col_pk_name);
	}
    }	
    $CLOSE get_pk_info;

    $FREE get_pk_info;
    
    g->pk_columns = strdup (all_cols);

    if (g->verbose > 1)
	printf ("%s: Primary key expression is <%s> key length %d\n",
		t->name, g->pk_columns, g->pk_column_len);

    return g->pk_column_count;
}

int get_server_nrows (db_thread *t, node *n)
{
    $int      nrows;
    $char     sql_stmt[BUF_SIZE];
    global   *g;

    g = t->globals;

    $SET PDQPRIORITY 100;

    sprintf (sql_stmt, "SELECT count(*) FROM %s@%s:%s",
	     g->database, n->name, g->table);

    $PREPARE get_sts_count_id FROM $sql_stmt;

    $EXECUTE get_sts_count_id INTO $nrows;
    
    $FREE get_sts_count_id;

    $SET PDQPRIORITY 0;
    
    if (g->verbose)
	printf ("%s: There are %d known TimeSeries on server %s\n",
		t->name, nrows, n->name);

    return nrows;
}

int get_total_nrows (db_thread *t)
{
    $int      nrows;
    $varchar *tabname;
    $char     sql_stmt[BUF_SIZE];
    char      grid_expr[IDENT_LEN+16] = "";
    global   *g;
    int       total_nrows;

    g = t->globals;

    /* If running in clustered mode we should already have the total from
       when the node information was gathered */
    if (g->grid && g->total_nrows > 0)
	return g->total_nrows;

    /* Otherwise use a grid query to check all servers */
    $SET PDQPRIORITY 100;

    if (g->meta_table)
	tabname = g->meta_table;
    else
	tabname = g->table;

    if (g->grid)
	sprintf (grid_expr, " GRID ALL '%s'", g->grid);
    
    sprintf (sql_stmt, "SELECT count(*) FROM %s%s", tabname, grid_expr);

    $PREPARE get_ts_count_id FROM $sql_stmt;

    $DECLARE get_ts_count CURSOR FOR get_ts_count_id;

    $OPEN get_ts_count;

    total_nrows = 0;
    while (sqlca.sqlcode == 0)
    {
	$FETCH get_ts_count INTO $nrows;

	if (sqlca.sqlcode == 0)
	{
	    total_nrows += nrows;
	}
    }
    $CLOSE get_ts_count;
    $FREE get_ts_count;
    $FREE get_ts_count_id;

    $SET PDQPRIORITY 0;
    
    if (g->verbose)
	printf ("%s: There are %d known TimeSeries\n",
		t->name, total_nrows);

    return total_nrows;
}

ts_data *store_ts_data (db_thread *t, int instanceid,
			node *n, char *cont_name, char *key, int nodeid,
			int hertz, ts_data *d)
{
    global    *g = t->globals;
    container *c = NULL;

    if (d == NULL)
	d = new_ts_data (key);

    if (n->next_ts_data + 1 > n->ts_data_list_size)
    {
	printf ("%s: Fatal: No more space in ts_data_list\n", t->name);
	exit (1);
    }
    
    if (cont_name)
	c = hash_search (&n->container_hash, cont_name);
    
    d->id = n->next_ts_data;
    d->ts_id = instanceid;
    d->nodeid_inserted = nodeid;
    d->nodeid_now = n->id;
    d->last_tstamp = 0;
    d->hertz = hertz;
    if (c)
    {
	c->refs++;
	if (g->verbose > 1)
	    printf ("%s: Container <%s> ref now %d\n", t->name,
		    c->name, c->refs);
	d->container = c->id;
    }
    else if (cont_name)
    {
	if (g->verbose)
	    printf ("%s: Missing container <%s>\n", t->name, cont_name);
    }
    n->ts_data_list[n->next_ts_data] = d;
    n->next_ts_data++;
    if (d->nodeid_now != d->nodeid_inserted)
	n->displaced++;
    
    return d;
}

char *key_match_expr (db_thread *t, int ncols, char *pk_columns, unsigned char *key)
{
    global *g = t->globals;
    int     klen = strlen (pk_columns) + strlen ((char *)key) +
	(ncols * 5) + ((ncols - 1) * 5) + 1;
    char   *expr = (char *) malloc (klen);
    char   *eptr = expr, *kptr = (char *)key, *kend;
    int     cols;

    for (cols = 0; cols < ncols; cols++)
    {
	kend = strchr (kptr, '|');
	if (kend == NULL)
	    klen = strlen (kptr);
	else
	    klen = kend - kptr;
	if (cols > 0)
	    eptr += sprintf (eptr, " and ");
	eptr += sprintf (eptr, "%s = '%*.*s'", g->pk_column_array[cols],
			 klen, klen, kptr);
	if (kend)
	    kptr = ++kend;
    }
    if (g->verbose > 1)
	printf ("key search <%s>\n", expr);
    
    return expr;
}

ts_data *load_key_info (db_thread *t, unsigned char *key, ts_data  *d)
{
    global   *g = t->globals;
    char     *match;
    int       count;
    node     *n;
    $varchar  cont_name[IDENT_LEN+1];
    $char     sql_stmt[BUF_SIZE];
    $long     instanceid = 0;
    $int      hertz = 0;

    n = &(g->server_list[t->connection->current_server]);
    
    match = key_match_expr (t, g->pk_column_count, g->pk_columns, key);
    
    sprintf (sql_stmt,
	     "SELECT TRIM(GetContainerName(%s)), GetHertz(%s) FROM %s WHERE %s",
	     g->column, g->column, g->table, match);

    free (match);

    if (g->verbose > 1)
	printf ("%s: EXEC-SQL %s;\n", t->name, sql_stmt);
    
    $PREPARE get_1ts_info_id FROM $sql_stmt;
    
    $DECLARE get_1ts_info CURSOR FOR get_1ts_info_id;
    
    $OPEN get_1ts_info;

    count = 0;
    while (sqlca.sqlcode == 0)
    {
	$FETCH get_1ts_info INTO $cont_name, $hertz;
	
	if (sqlca.sqlcode == 0)
	{
	    count++;
	    d = store_ts_data (t, instanceid, n, cont_name, (char *)key, 0, hertz, d);
	}
    }	
    $CLOSE get_1ts_info;
    
    $FREE get_1ts_info;
    $FREE get_1ts_info_id;
    
    if (count > 1 && g->verbose)
	printf ("%s: Unexpected - got %d rows from get_1ts_info\n",
		t->name, count);

    return d;
}

ts_data *insert_new_ts (db_thread *t, unsigned char *key, char *ts_spec, ts_data *d)
{
    global    *g = t->globals;
    $char      sql_stmt[BUF_SIZE];
    char      *sptr, *kptr = (char *)key, *kend;
    int        col, klen;

    sptr = sql_stmt;
    sptr += sprintf (sptr, "INSERT INTO %s (", g->table);

    for (col = 0; col < g->pk_column_count; col++)
    {
	if (col > 0)
	    sptr += sprintf (sptr, ", ");
	sptr += sprintf (sptr, "%s", g->pk_column_array[col]);
    }
    sptr += sprintf (sptr, ", %s) values ('", g->column);
    for (col = 0; col < g->pk_column_count; col++)
    {
	kend = strchr (kptr, '|');
	if (kend == NULL)
	    klen = strlen (kptr);
	else
	    klen = kend - kptr;
	if (col > 0)
	    sptr += sprintf (sptr, ", ");
	sptr += sprintf (sptr, "%*.*s'", klen, klen, kptr);
	kptr += (klen + 1);
    }
    sprintf (sptr, ", '%s')", ts_spec);

    if (g->verbose)
	printf ("insert_new_ts(): EXEC-SQL <%s>\n", sql_stmt);

    $whenever error continue;
    
    $execute immediate $sql_stmt;

    if (sqlca.sqlcode != 0 && g->verbose)
	printf ("ignore error %d from INSERT\n", sqlca.sqlcode);
    
    $whenever error call SQL_Check;
    
    return load_key_info (t, key, d);
}

int get_keys (db_thread *t)
{
    $varchar  all_cols[COL_BUF_SIZE];
    $varchar  cont_name[IDENT_LEN+1];
    $char     sql_stmt[BUF_SIZE];
    $long     instanceid;
    $int      hertz, nodeid;

    node     *n;
    global   *g;

    $SET PDQPRIORITY 100;

    g = t->globals;

    n = &(g->server_list[t->connection->current_server]);
    n->next_ts_data = 0;

    if (g->meta_table)
    {	
	/* Urgh - It's not possible to join a grid table to a regular table.
	   Also not possible to select a remote TimeSeries. Kak! */
	
	if (g->grid)
	{
	    sprintf (sql_stmt,
		 "SELECT %s dname, %s.%s did, %s.%s ts_node, ifx_node_id() dev_node  FROM %s GRID ALL '%s' WHERE %s.%s = %d INTO TEMP id_table WITH NO LOG",
		     g->pk_columns,
		     g->meta_table, g->meta_join,
		     g->meta_table, g->meta_nodeid,
		     g->meta_table, g->grid,
		     g->meta_table, g->meta_nodeid, n->id);
	    
	    if (g->verbose > 1)
		printf ("%s: EXEC-SQL %s;\n", t->name, sql_stmt);
	    
	    $EXECUTE IMMEDIATE $sql_stmt;
	    
	    sprintf (sql_stmt,
		     "SELECT dname, did, dev_node, TRIM(GetContainerName(%s.%s)), GetHertz(%s.%s) FROM %s, id_table WHERE id_table.did = %s.%s",
		     g->table, g->column,
		     g->table, g->column,
		     g->table, 
		     g->table, g->meta_join);
	}
	else
	{
	    sprintf (sql_stmt,
		     "SELECT %s, %s.%s, %s.%s, TRIM(GetContainerName(%s.%s)), GetHertz(%s.%s) FROM %s@%s:%s, %s WHERE %s.%s = %s.%s",
		     g->pk_columns,
		     g->table, g->meta_join,
		     g->meta_table, g->meta_nodeid,
		     g->table, g->column,
		     g->table, g->column,
		     g->database, n->name, g->table, g->meta_table,
		     g->meta_table, g->meta_join, g->table, g->meta_join);
	}
    }
    else
    {
	sprintf (sql_stmt,
		 "SELECT %s, 0, 0, TRIM(GetContainerName(%s)), GetHertz(%s) FROM %s",
		 g->pk_columns, g->column, g->column, g->table);
    }
    if (g->verbose > 1)
	printf ("%s: EXEC-SQL %s;\n", t->name, sql_stmt);
    
    $PREPARE get_ts_info_id FROM $sql_stmt;
    
    $DECLARE get_ts_info CURSOR FOR get_ts_info_id;
    
    $OPEN get_ts_info;
    
    while (sqlca.sqlcode == 0)
    {
	$FETCH get_ts_info
	    INTO $all_cols, $instanceid, $nodeid, $cont_name, $hertz;
	
	if (sqlca.sqlcode == 0)
	{
	    store_ts_data (t, instanceid, n, cont_name, all_cols, nodeid, hertz, NULL);
	}
	
    }	
    $CLOSE get_ts_info;
    
    $FREE get_ts_info;
    $FREE get_tsinfo_id;
    
    $SET PDQPRIORITY 0;

    if (g->verbose)
	printf ("%s: Retrieved %d of %d active TimeSeries from node %s, %d have moved\n",
		t->name, n->next_ts_data, n->nrows, n->name, n->displaced);
    
    return n->next_ts_data;
}

int db_flush_data (db_thread *t)
{
    $int      rval = 0, flag, i, txsize;
    $varchar *s_handle;
    $db_prep *p = t->connection->prepared;

    node     *n;
    global   *g;
    struct timeval start_tv;

    gettimeofday (&start_tv, NULL);
    
    g = t->globals;
    n = &(g->server_list[t->connection->current_server]);
    
    flag = g->flushflag;
    s_handle = n->handle;
    
    if (g->reduced_log)
	flag |= TSOPEN_REDUCED_LOG;

    $WHENEVER ERROR CALL SQL_Check;

    if (g->lock_containers || p->commit_data_id == NULL)
    {
	$BEGIN;

	if (t->container_locks)
	    for (i = 0; i < t->ncontainers; i++)
	    {
		container *c = t->container_list[i];
		if (c->lock)
		{
		    if (g->verbose > 1)
			printf ("%s: Lock container id %d <%s>\n", t->name,
				i, c->name);
		    lock_container (t, c->name);
		}
		c->lock = 0;
	    }

	$EXECUTE $p->flush_data_id INTO $rval USING $s_handle, $flag;

	$COMMIT;

	/* This is not set by the info function as the commit occurs
	   outside the scope of the flush. */
	t->commits++;
    }
    else
    {
	$short txsizeind = 0;
	
	if ((txsize = g->txsize) == -1)
	    txsizeind = -1;

	$EXECUTE $p->commit_data_id INTO $rval
	    USING $s_handle, $flag, $txsize:txsizeind;
    }

    if (g->collect_info)
    {
	$int  containers, elements, duplicates, instance_ids,
	    commits, rollbacks, exceptions, errors;

	$EXECUTE $p->info_id
	    INTO $containers, $elements, $duplicates, $instance_ids,
	    $commits, $rollbacks, $exceptions, $errors USING $s_handle;
	
	t->containers += containers;
	t->elements += elements;
	t->duplicates += duplicates;
	t->instance_ids += instance_ids;
	t->commits += commits;
	t->rollbacks += rollbacks;
	t->exceptions += exceptions;
	t->errors += errors;
    }

    gettimeofday (&(t->last_flush_tv), NULL);

    timersub (&(t->last_flush_tv), &start_tv, &(t->flush_elapsed));
    timerinc (&(t->total_flush_elapsed), &(t->flush_elapsed));
    
    return rval;
}

int tsl_release (db_thread *t)
{
    $int      response;
    $varchar *s_handle = t->globals->server_list[t->connection->current_server].handle;
    $db_prep *p = t->connection->prepared;

    $WHENEVER ERROR CALL SQL_Check;
	
    $EXECUTE $p->release_id INTO $response USING $s_handle;

    return response;
}

int tsl_shutdown (db_thread *t)
{
    $int      response;
    $varchar *s_handle = t->globals->server_list[t->connection->current_server].handle;
    $db_prep *p = t->connection->prepared;

    response = tsl_release (t);

    $WHENEVER ERROR CALL SQL_Check;
	
    $EXECUTE $p->shutdown_id USING $s_handle;

    return response;
}

void free_stmt_id (char **id)
{
    $varchar *stmt_id = *id;

    $FREE $stmt_id;
}

void free_id (char **id)
{
    if (id && *id)
    {
	free (*id);
	*id = NULL;
    }
}

void free_connection (db_conn *c)
{
    db_prep   *p = c->prepared;
    
    switch (c->conn_type)
    {
    case T_UNKNOWN:
	break;

    case T_MAIN:
	free_id (&p->init_table_id);
	free_id (&p->release_id);
	free_id (&p->shutdown_id);
	free_id (&p->connection_id);
	break;
	
    case T_LOADER:
	free_id (&p->attach_id);
	free_id (&p->put_data_id);
	free_id (&p->release_id);
	free_id (&p->lock_container_id);
	free_id (&p->flush_data_id);
	free_id (&p->commit_data_id);	
	free_id (&p->info_id);
	free_id (&p->connection_id);
	break;
    case T_STORAGE:
	free_id (&p->storage_id);
	break;
    case T_REWRITE:
	free_id (&p->next_device_id);
	break;
    case T_PROVISION:
	break;
    default:
	break;
    }

    free (c);    
}

int db_disconnect (db_thread *t)
{
    db_conn   *c = t->connection;
    $db_prep  *p = c->prepared;

    $WHENEVER ERROR CALL SQL_Check;

    if (c->connected == 0)
	return 0;
    
    switch (c->conn_type)
    {
    case T_UNKNOWN:
	break;
    case T_MAIN:
	free_stmt_id (&p->init_table_id);
	free_stmt_id (&p->release_id);
	free_stmt_id (&p->shutdown_id);
	break;	
    case T_LOADER:
	free_stmt_id (&p->attach_id);
	free_stmt_id (&p->put_data_id);
	free_stmt_id (&p->release_id);
	free_stmt_id (&p->lock_container_id);
	free_stmt_id (&p->flush_data_id);
	free_stmt_id (&p->commit_data_id);	
	free_stmt_id (&p->info_id);
	break;
    case T_STORAGE:
	free_stmt_id (&p->storage_id);
	break;
    case T_REWRITE:
	free_stmt_id (&p->next_device_id);
	break;
    case T_PROVISION:
	break;
    default:
	break;
    }
	
    $DISCONNECT $p->connection_id;

    c->connected = 0;

    return 0;
}

int db_insert_storage (db_thread *t,
		       EXEC SQL BEGIN DECLARE SECTION;
		       parameter varchar *pk,
		       parameter varchar *tstamp,
		       EXEC SQL END DECLARE SECTION;
		       char *values)
{

    $int      val = strtol (values, NULL, 0);
    $db_prep *p = t->connection->prepared;

    if (t->globals->verbose > 0)
	printf ("%s; Inserting (%s, %s, %d)\n", t->name, pk, tstamp, val);
		
    $EXECUTE $p->storage_id USING $pk, $tstamp, $val;

    return sqlca.sqlcode;
}

long db_next_device (
    db_thread *t,
    EXEC SQL BEGIN DECLARE SECTION;
    parameter varchar *container,
    parameter short hertz
    EXEC SQL END DECLARE SECTION;
    )
{
    $long      s_devid;
    $db_prep  *p = t->connection->prepared;

    if (t->globals->verbose > 0)
	printf ("%s; Getting next_device()\n", t->name);
		
    $EXECUTE $p->next_device_id INTO $s_devid, $container, $hertz;

    return s_devid;
}

long db_next_displaced (db_thread *t, char *key)
{
    $long      s_devid;
    $varchar   stmt[BUF_SIZE];
    $varchar  *s_key = key;
    global    *g;

    g = t->globals;
    
    if (g->verbose > 0)
	printf ("%s; Getting next_device()\n", t->name);
		
    sprintf (stmt, "SELECT FIRST 1 deviceid, devicename FROM %s WHERE %s != %d",
	     g->meta_table, g->meta_nodeid, g->server_list[t->connection->current_server].id);

    if (g->verbose > 0)
	printf ("%s: EXEC-SQL %s\n", t->name, stmt);

    $PREPARE stmt_id FROM $stmt;
    
    $EXECUTE stmt_id INTO $s_devid, $s_key;

    if (g->verbose > 0)
	printf ("%s: Next device to be moved %ld\n", t->name, s_devid);
    
    return s_devid;
}

int db_move_ts (db_thread *t, long old_id, long new_id, int server)
{
    $char     s_sql[BUF_SIZE];

    global     *g;
    node       *n;

    g = t->globals;
    n = &(g->server_list[server]);
    
    sprintf (s_sql, "EXECUTE FUNCTION transfer_ts (%ld, '%s', %ld)",
	     new_id, n->name, old_id);

    if (g->verbose > 0)
	printf ("%s: EXEC-SQL %s;\n", t->name, s_sql);
    
    $WHENEVER ERROR CALL SQL_Check;

    return 0;
}
