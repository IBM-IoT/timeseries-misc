/***************************************************************************
 *
 * Licensed Materials - Property of IBM
 * 
 * Restricted Materials of IBM 
 * 
 * IBM Informix Dynamic Server
 * (c) Copyright IBM Corporation 2011, 2013 All rights reserverd.
 *
 *
 *  Title            : sql_defs.h 
 *  Description      : Header file for TimeSeries Cache Loader - SQL defs
 *
 ****************************************************************************
 */

$ifndef SQL_DEFS_H;
$define SQL_DEFS_H 1;

/* Prepared statement information */
$struct db_prep_s {

    /* Admin thread statement ids*/
    char *init_table_id;
    char *shutdown_id;  
    char *storage_id;
    char *next_device_id;
  
    /* Loader thread statement ids */
    char *attach_id;
    char *put_data_id;
    char *put_sql_id;
    char *flush_data_id;
    char *commit_data_id;
    char *lock_container_id;
    char *info_id;

    /* Shared statement ids */
    char *connection_id;
    char *release_id;
};

$typedef struct db_prep_s db_prep;

$endif;
