/***************************************************************************
 *
 * Licensed Materials - Property of IBM
 * 
 * Restricted Materials of IBM 
 * 
 * IBM Informix Dynamic Server
 * (c) Copyright IBM Corporation 2011, 2015 All rights reserverd.
 *
 *
 *  Title            : defs.h 
 *  Description      : Header file for TimeSeries Cache Loader
 *
 ****************************************************************************
 */

#ifndef DEFS_H
#define DEFS_H 1

#ifndef AIX
#define HAVE_pthread_setname_np
#define HAVE_prctl
#endif

#ifndef IDENT_LEN
#define IDENT_LEN 128
#endif

#define MAX_LVARCHAR           32739
#define LINE_SIZE	       1024
#define BUF_SIZE	       MAX_LVARCHAR
#define MIN_CONTAINER_TABLE    500
#define MIN_TS_DATA_SIZE       5000
#define DEFAULT_TXSIZE         5000
#define DEFAULT_DTFORMAT       "%Y-%m-%d %H:%M:%S"
#define DEFAULT_LOCKWAIT       30
#define DEFAULT_REPEAT_MARKER  100
#define DEFAULT_FLUSH_INTERVAL 30

#define INPUT_DIR    "Input"
#define PROCESS_DIR  "Process"
#define COMPLETE_DIR "Complete"
#define FILENAME_LEN 1024

#define CMD_HDR            -1
#define CMD_TERMINATE      -2
#define CMD_NEW_CONTAINER  -3
#define CMD_FLUSH          -4

extern  int end_process;

/* Thread types */

#define IS_T_ADMIN(__t)	  ((__t) == T_MAIN)
#define IS_T_DATA(__t)    (((__t) == T_REWRITE)||((__t) == T_STORAGE))

/* Status of device */
#define MOVE_BASE_ROWS    1
#define COLLECT_NEW_ROWS  2

enum t_type_e {
    T_UNKNOWN = 0, T_MAIN, T_STATUS, T_READER, T_DIR_READER,
    T_LOADER, T_REWRITE, T_STORAGE, T_PROVISION
};

typedef enum t_type_e t_type;

/* Conection information */
struct db_conn_s {

    /* Identify this connection */
    int    current_server;
    int    conn_id;
    t_type conn_type;
    int    connected;

    /* This is SQL data, defined in a separate $include file */
    struct db_prep_s *prepared;
};

typedef struct db_conn_s db_conn;

typedef struct container_s {

    char  *name;
    int    id;
    int    refs;
    int    queue;
    int    lock;
    struct timeval last_put_tv;

} container;

typedef struct ts_data_s {

    char  *key;
    int    id;
    int    container;
    short  nodeid_now;
    short  nodeid_inserted;
    time_t last_tstamp;
    short  hertz;
    long   ts_id;
    long   new_id;
    int    status;
    char  *last_out;
    int    last_buf_len;
    int    last_out_len;
    int    repeat_count;
    
} ts_data;


typedef struct hdr_s {

    int    len;
    int    dev_id;
    long   ts_id;

} hdr_t;

typedef struct node_s {

    int         id;
    char       *handle;
    char       *name;
    container **container_list;
    int         container_hash_total;
    int         container_table_size;
    int         dev_nrows;
    int         nrows;
    int        *pipelist;
    int         nqueues;
    int         displaced;
    int         storagefd;
    int         rewritefd;
    int         next_ts_data;
    int         provisionfd;
    struct db_thread_s  *provision_thread;
    struct db_thread_s  **threadlist;
    ts_data   **ts_data_list;
    int         ts_data_list_size;
    HASH_HDR    container_hash;

} node;


/* Global data - should be readonly after initialisation */
typedef struct globals_s {

    /* Args passed in from main */
    char  *database;
    char  *table;
    char  *user;
    char  *password;
    char  *column;
    char  *rejects;
    char  *tsl_rejects;
    char  *logfile;
    char  *dtformat;
    char  *where;
    char  *input;
    char  *isolation;
    char  *input_dir;
    char  *putbuf_dir;
    char   ifs;
    char  *new_ts;
    char  *servers;
    int    txsize;
    int    status_update;
    int    collect_info;
    int    put_buffer;
    int    put_count;
    int    reduced_log;
    int    verbose;
    int    lock_containers;
    int    to_utc;
    int    pass_through;
    int    nthreads;
    int    in_threads;
    int    set_nthreads;
    int    lock_wait;
    int    total_nrows;
    int    flushflag;
    struct timeval flush_interval_tv;
    int    total_queues;
    int    unique_id;
    int    skip_cols;
    int    remove_all_nulls;
    int    fix_non_ascii;
    int    repeat_marker;
    int    one_shot;

    /* Command & Control */
    int    statusfd;
    int    readerfd;
    int   *dir_readerfd;
    struct db_thread_s **dir_reader;
    
    /* Cluster info */
    int      nservers;
    char    *grid;
    node    *server_list;
    char    *meta_table;
    char    *meta_join;
    char    *meta_nodeid;

    /* Hash table data */
    HASH_HDR            pk_hash;
    char               *pk_columns;
    char               *pk_column_array[16];
    int                 pk_column_count;
    int                 pk_column_len;
    int                 pk_hash_total;
    char               *key_match;

    /* JSON formatting */
    char **json_key_names;
    char  *inputformat;
    int    json_ncols;

} global;


/* All thread local storage */
typedef struct db_thread_s {

    /* The tid is the place in the list and may be reused,
       The unique_id is different for every thread */
    int                 tid;
    int                 unique_id;
    t_type              type;
    pthread_t           thread;
    struct db_thread_s *main;
    char               *name;
    char               *putbuf_file;
    int                 pipe;
    void             *(*pfunc)(void *);
    global             *globals;
    db_conn            *connection;
    char               *buffer;
    char               *bptr;
    
    int         nlocked;
    int         container_locks;
    int         treturn;
    int         mod_queue;
    int         ncontainers;
    container **container_list;

    /* Counters from TSL_FlushInfo */
    int containers;
    int elements;
    int duplicates;
    int instance_ids;
    int commits;
    int rollbacks;
    int exceptions;
    int errors;

    /* Other counters */
    int nflush;
    int nput;
    int nint;
    int nallnulls;
    int rejects;
    int puts_this_flush;

    /* Time based metrics */
    struct timeval put_elapsed;
    struct timeval flush_elapsed;
    struct timeval last_put_tv;
    struct timeval last_flush_tv;
    struct timeval total_put_elapsed;
    struct timeval total_flush_elapsed;

} db_thread;

#include "proto.h"

#endif
