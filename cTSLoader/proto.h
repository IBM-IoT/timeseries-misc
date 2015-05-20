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
 *  Title            : proto.h 
 *  Description      : Function prototypes for TimeSeries Cache Loader
 *
 ****************************************************************************
 */

#ifndef PROTO_H
#define PROTO_H 1

/* From database.ec */
int db_connect (db_thread *t, int node, t_type type);
int db_disconnect (db_thread *t);
int db_server_list (db_thread *t);
int db_grid_list (db_thread *t);
int table_attach (db_thread *t);
int get_pk_info (db_thread *t);
int get_hashkeys (db_thread *t);
int get_containers (db_thread *t);
int get_keys (db_thread *t);
int get_cluster_info (db_thread *t);
int get_total_nrows (db_thread *t);
int table_init (db_thread *t);
int db_put_data (db_thread *t, char *data);
int lock_container_sql (db_thread *t, char *container, int lock);
int db_flush_data (db_thread *t);
int tsl_release (db_thread *t);
int tsl_shutdown (db_thread *t);
int tsl_disconnect (db_thread *t);
long db_next_device (db_thread *t, char *container, short hertz);
int db_insert_storage (db_thread *t, char *pk, char *tstamp, char *values);
long db_next_displaced (db_thread *t, char *key);
int db_move_ts (db_thread *t, long new_id, long old_id, int server);
void db_get_cpu_info (db_thread *t);
db_conn *create_connection (int node, int type, int id);
void free_connection (db_conn *c);
ts_data *load_key_info (db_thread *t, unsigned char *key, ts_data *d);
ts_data *insert_new_ts (db_thread *t, unsigned char *key, char *ts_spec, ts_data *d);
ts_data *store_ts_data (db_thread *t, int instanceid, node *n, char *cont_name, char *key, int, int hertz, ts_data *d);
container *store_container (char *cname, node *n);
int get_server_nrows (db_thread *t, node *n);


/* From threads.c - entry point for threaded functions */
/* These functions actual take a db_thread * as an argument but
   that doesn't work for pthread_create() */
void *status      (void *t);
void *reader      (void *t);
void *loader      (void *t);
void *init        (void *t);
void *provision   (void *t);
void *hashlists   (void *t);
void *rewrite     (void *t);
void *storage     (void *t);

/* From process.c */
int   find_server (db_thread *t, int nodeid);
db_thread *find_thread (int type);
void  flush_all   (db_thread *t);
int write_pipe (db_thread *t, int piped, char *buf, int len);
int sleepfor (int nsec);
void shutdown_all (db_thread *t);
int get_message (db_thread *t, hdr_t *hdr, char *line);
void keycpy (char *key, char *line, int ncols, char ifs);
int log_reject (char *reject_file, char *reject_line);
int json_format (char *line_in, char *line_out, char **keys, int ncols);
int inputlen (unsigned char *in, int fix, int *reject);
int keycmp (char *key, char *line, int ncols, char ifs);
time_t parse_line (char *in, int len, int tstamp_col, char ifs,
		   time_t *last_local_time, char **ts_start,
		   int *p_year, int *p_month, int *p_day);
int remove_repeats (char *line, int inlen, ts_data *d, char ifs, 
		    int start_col, int skip_cols, int remove_all_nulls,
		    int repeat_marker);
int put_buffer (db_thread *t);
void flush_data (db_thread *t);
void send_cmd_queue (db_thread *t, hdr_t *hdr, node *n, int queue);
void send_cmd_node (db_thread *t, hdr_t *hdr, node *n);
void send_cmd_all (db_thread *t, hdr_t *hdr);
void send_flush_thread (db_thread *t, node *n, int queue);
void send_flush_node (db_thread *t, node *n);
void send_flush_all (db_thread *t);

/* From hash.c */
int   hash_create (HASH_HDR *hash_table, size_t n_entries);
void  hash_destroy (HASH_HDR *hash_table);
void *hash_insert (HASH_HDR *hash_table, char *key, void *data);
void *hash_search (HASH_HDR *hash_table, char *key);

/* From json.c */
int json_parse_string_pairs (char *json, char ***key_strings, char ***value_strings);
int json_keyncmp (char *k1, char *k2, int len);
int json_lookup (char *lbrace, int len, char *key, char *value);

/* From main.c */
char *t_name (int *id);
db_thread *init_thread (db_thread *t, t_type type);
int next_file_in_dir (char *dirname, char *filter, char *filename);
void timerinc (struct timeval *total, struct timeval *incr);
#ifndef timeradd
void timeradd (struct timeval *a, struct timeval *b, struct timeval *res);
#endif
#ifndef timersub
void timersub (struct timeval *a, struct timeval *b, struct timeval *res);
#endif
extern db_thread **tlist;
extern int thread_total;
extern int thread_block_size;

/* From usage.c */
void usage (char *progname);
void options (void);
void man (void);
void version (void);

/* From ecp_chk.ec */
void whenexp_chk_line(int _line_no_);
void _dummy(void);

#endif
