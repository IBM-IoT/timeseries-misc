#include <string.h>
#include <sqlhdr.h>
#include <sqliapi.h>
static const char _Cn16[] = "stmt_id";
static const char _Cn15[] = "get_tsinfo_id";
static const char _Cn14[] = "get_ts_info";
static const char _Cn13[] = "get_ts_info_id";
static const char _Cn12[] = "get_1ts_info";
static const char _Cn11[] = "get_1ts_info_id";
static const char _Cn10[] = "get_ts_count";
static const char _Cn9[] = "get_ts_count_id";
static const char _Cn8[] = "get_sts_count_id";
static const char _Cn7[] = "get_pk_info";
static const char _Cn6[] = "get_container";
static const char _Cn5[] = "get_nrows";
static const char _Cn4[] = "get_nrows_id";
static const char _Cn3[] = "get_nodes";
static const char _Cn2[] = "get_nodes_id";
static const char _Cn1[] = "cpus_id";
void _dummy_SD0(ifx_sqlda_t *_SD0) {return;};
#line 1 "database.ec"
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

/* 
 * $define IDENT_LEN 128;
 */
#line 29 "database.ec"

/* 
 * $define MAX_NODES 255;
 */
#line 30 "database.ec"

/* This needs to be big enough to hold the longest concatenated primary key.
   Maximum ley length is based on page size. Minimum is 390 for a 2K page. */
/* 
 * $define COL_BUF_SIZE (390*4)+16;
 */
#line 33 "database.ec"


/* 
 * $include "sql_defs.h";
 */
#line 35 "database.ec"

#line 35 "database.ec"
#line 1 "sql_defs.h"
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

/* 
 * $ifndef SQL_DEFS_H;
 */
#line 17 "sql_defs.h"

/* 
 * $define SQL_DEFS_H 1;
 */
#line 18 "sql_defs.h"


/* Prepared statement information */
/*
 * $struct db_prep_s {
 * 
 * 
 *     char *init_table_id;
 *     char *shutdown_id;
 *     char *storage_id;
 *     char *next_device_id;
 * 
 * 
 *     char *attach_id;
 *     char *put_data_id;
 *     char *put_sql_id;
 *     char *flush_data_id;
 *     char *commit_data_id;
 *     char *lock_container_id;
 *     char *info_id;
 * 
 * 
 *     char *connection_id;
 *     char *release_id;
 * };
 */
#line 21 "sql_defs.h"
struct db_prep_s
  {
      char *init_table_id;
      char *shutdown_id;
      char *storage_id;
      char *next_device_id;
      char *attach_id;
      char *put_data_id;
      char *put_sql_id;
      char *flush_data_id;
      char *commit_data_id;
      char *lock_container_id;
      char *info_id;
      char *connection_id;
      char *release_id;
  } ;

/*
 * $typedef struct db_prep_s db_prep;
 */
#line 43 "sql_defs.h"
typedef struct db_prep_s db_prep;

/* 
 * $endif;
 */
#line 45 "sql_defs.h"

#line 45 "sql_defs.h"
#line 36 "database.ec"

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

/* 
 * $include sqlstype;
 */
#line 56 "database.ec"

#line 56 "database.ec"
#line 1 "/home/informix/1210FC5/incl/dmi/sqlstype.h"
/****************************************************************************
 *
 *  Licensed Material - Property Of IBM
 *
 *  "Restricted Materials of IBM"
 *
 *  IBM Informix Client SDK
 *  Copyright IBM Corporation 1997, 2012
 *
 *  Title:       sqlstype.h
 *  Description: Defined symbols for SQL statement types
 *
 ***************************************************************************
 */

#ifndef SQLSTYPE_H_INCLUDED
#define SQLSTYPE_H_INCLUDED

/*
 * SQL statement types
 */

#define SQ_DATABASE	1	/* DATABASE              */
#define SQ_SELECT	2	/* SELECT		 */
#define SQ_SELINTO	3	/* SELECT INTO           */
#define SQ_UPDATE	4	/* UPDATE...WHERE        */
#define SQ_DELETE	5	/* DELETE...WHERE        */
#define SQ_INSERT	6	/* INSERT                */
#define SQ_UPDCURR	7	/* UPDATE WHERE CURRENT OF */
#define SQ_DELCURR	8	/* DELETE WHERE CURRENT OF */
#define SQ_LDINSERT	9	/* for internal use only */
#define SQ_LOCK		10	/* LOCK TABLE            */
#define SQ_UNLOCK	11	/* UNLOCK TABLE          */
#define SQ_CREADB	12	/* CREATE DATABASE       */
#define SQ_DROPDB	13	/* DROP DATABASE         */
#define SQ_CRETAB	14	/* CREATE TABLE          */
#define SQ_DRPTAB	15	/* DROP TABLE            */
#define SQ_CREIDX	16	/* CREATE INDEX          */
#define SQ_DRPIDX	17	/* DROP INDEX            */
#define SQ_GRANT	18	/* GRANT                 */
#define SQ_REVOKE	19	/* REVOKE                */
#define SQ_RENTAB	20	/* RENAME TABLE          */
#define SQ_RENCOL	21	/* RENAME COLUMN         */
#define SQ_CREAUD	22	/* CREATE AUDIT          */
#define SQ_STRAUD	23	/* for internal use only */
#define SQ_STPAUD	24	/* for internal use only */
#define SQ_DRPAUD	25	/* DROP AUDIT            */
#define SQ_RECTAB	26	/* RECOVER TABLE         */
#define SQ_CHKTAB	27	/* for internal use only */
#define SQ_REPTAB	28	/* for internal use only */
#define SQ_ALTER	29	/* ALTER TABLE           */
#define SQ_STATS	30	/* UPDATE STATISTICS     */
#define SQ_CLSDB	31	/* CLOSE DATABASE        */
#define SQ_DELALL	32	/* DELETE (no WHERE)     */
#define SQ_UPDALL	33	/* UPDATE (no WHERE)     */
#define SQ_BEGWORK	34	/* BEGIN WORK            */
#define SQ_COMMIT	35	/* COMMIT WORK           */
#define SQ_ROLLBACK	36	/* ROLLBACK WORK         */
#define SQ_SAVEPOINT	37	/* for internal use only */
#define SQ_STARTDB	38	/* START DATABASE        */
#define SQ_RFORWARD	39	/* ROLLFORWARD DATABASE  */
#define SQ_CREVIEW	40	/* CREATE VIEW           */
#define SQ_DROPVIEW	41	/* DROP VIEW             */
#define SQ_DEBUG	42	/* for internal use only */
#define SQ_CREASYN	43	/* CREATE SYNONYM        */
#define SQ_DROPSYN	44	/* DROP SYNONYM          */
#define SQ_CTEMP	45	/* CREATE TEMP TABLE     */
#define SQ_WAITFOR	46	/* SET LOCK MODE         */
#define SQ_ALTIDX       47	/* ALTER INDEX           */
#define SQ_ISOLATE	48	/* SET ISOLATION         */
#define SQ_SETLOG	49	/* SET LOG               */
#define SQ_EXPLAIN	50	/* SET EXPLAIN           */
#define SQ_SCHEMA	51	/* CREATE SCHEMA         */
#define SQ_OPTIM	52	/* SET OPTIMIZATION      */
#define SQ_CREPROC	53	/* CREATE PROCEDURE      */
#define SQ_DRPPROC	54	/* DROP PROCEDURE        */
#define SQ_CONSTRMODE   55	/* SET CONSTRAINTS       */
#define SQ_EXECPROC	56	/* EXECUTE PROCEDURE     */
#define SQ_DBGFILE	57	/* SET DEBUG FILE TO     */
#define SQ_CREOPCL	58	/* CREATE OPTICAL CLUSTER */
#define SQ_ALTOPCL	59	/* ALTER OPTICAL CLUSTER */
#define SQ_DRPOPCL	60	/* DROP OPTICAL CLUSTER  */
#define SQ_OPRESERVE	61	/* RESERVE (optical)     */
#define SQ_OPRELEASE	62	/* RELEASE (optical)     */
#define SQ_OPTIMEOUT	63	/* SET OPTICAL TIMEOUT  */
#define SQ_PROCSTATS    64	/* UPDATE STATISTICS (for procedure) */

/* 65 and 66 were used for SQ_GRANTGRP and SQ_REVOKGRP for KANJI
 * their functionality is replaced by ROLE
 */

/* 67, 68 and 69 were used for SQ_SKINHIBIT, SQ_SKSHOW and SQ_SKSMALL
 * which are no longer supported
 */

#define SQ_CRETRIG	70	/* CREATE TRIGGER        */
#define SQ_DRPTRIG	71	/* DROP TRIGGER          */

/*
 * This statement type is reserved for identifying new statements with
 * custom syntax extensions to the generic SQL parser
 */
#define SQ_UNKNOWN	72
#define SQ_SETDATASKIP	73	/* SET DATASKIP          */
#define SQ_PDQPRIORITY  74	/* SET PDQPRIORITY       */
#define SQ_ALTFRAG	75	/* ALTER FRAGMENT        */

#define SQ_SETOBJMODE   76      /* SET MODE ENABLED/DISABLED/FILTERING   */
#define SQ_START        77      /* START VIOLATIONS TABLE   */
#define SQ_STOP         78      /* STOP VIOLATIONS TABLE   */

#define SQ_SETMAC       79      /* SET SESSION LEVEL */
#define SQ_SETDAC       80      /* SET SESSION AUTHORIZATION */
#define SQ_SETTBLHI     81	/* SET TABLE HIGH */
#define SQ_SETLVEXT     82	/* SET EXTENT SIZE */

#define SQ_CREATEROLE   83	/* CREATE ROLE */
#define SQ_DROPROLE     84  	/* DROP ROLE */
#define SQ_SETROLE      85	/* SET ROLE */
#define SQ_PASSWD       86	/* SET DBPASSWORD */

#define SQ_RENDB	87	/* RENAME DATABASE */

#define SQ_CREADOM	88	/* CREATE DOMAIN */
#define SQ_DROPDOM	89	/* DROP DOMAIN   */

#define SQ_CREANRT	90	/* CREATE NAMED ROW TYPE */
#define SQ_DROPNRT	91	/* DROP NAMED ROW TYPE   */

#define SQ_CREADT       92      /* CREATE DISTINCT TYPE */
#define SQ_CREACT       93      /* CREATE CAST */
#define SQ_DROPCT       94      /* DROP CAST */

#define SQ_CREABT       95      /* CREATE OPAQUE TYPE */
#define SQ_DROPTYPE     96      /* DROP TYPE */

#define SQ_ALTERROUTINE 97      /* ALTER routine */

#define SQ_CREATEAM	98	/* CREATE ACCESS_METHOD */
#define SQ_DROPAM	99	/* DROP ACCESS_METHOD   */
#define SQ_ALTERAM     100	/* ALTER ACCESS_METHOD   */

#define SQ_CREATEOPC   101	/* CREATE OPCLASS */
#define SQ_DROPOPC     102	/* DROP OPCLASS   */

#define SQ_CREACST     103      /* CREATE CONSTRUCTOR */

#define SQ_SETRES      104     /* SET (MEMORY/NON)_RESIDENT */

#define SQ_CREAGG      105	/* CREATE AGGREGATE */
#define SQ_DRPAGG      106	/* DROP AGGREGATE   */

#define SQ_PLOADFILE	107	/* pload log file command*/
#define SQ_CHKIDX	108	/* onutil check index command */
#define SQ_SCHEDULE	109      /* set schedule          */
#define SQ_SETENV       110      /* "set environment ..." */

#define SQ_CREADUP      111     /* create duplicate - 8.31 xps */
#define SQ_DROPDUP      112     /* drop duplicate - 8.31 xps */
#define SQ_XPS_RES4	113	/* reserved for future use */
#define SQ_XPS_RES5	114	/* reserved for future use */

#define SQ_STMT_CACHE   115     /* SET STMT_CACHE */

#define SQ_RENIDX       116     /* RENAME INDEX   */
#define SQ_CREAGUID	117	/* Create ServerGuid */
#define SQ_DROPGUID	118	/* Drop ServerGuid */
#define SQ_ALTRGUID	119	/* Alter ServerGuid */
#define SQ_ALTERBT	120	/* ALTER TYPE */

#define SQ_ALTERCST	121	/* ALTER CONSTRUCTOR */
#define SQ_TRUNCATE	122     /* truncate table   */

#define SQ_IMPLICITTX   123     /* Implicit transaction */
#define SQ_CRESEQ       124     /* CREATE SEQUENCE*/
#define SQ_DRPSEQ       125     /* DROP SEQUENCE  */
#define SQ_ALTERSEQ     126     /* ALTER SEQUENCE */
#define SQ_RENSEQ       127     /* RENAME SEQUENCE */
#define SQ_MERGE        128     /* MERGE statement */
#define SQ_MVTAB	129	/* MOVE TABLE ... TO DATABASE ... */
#define SQ_EXTD         133     /*save external directive*/
#define SQ_CRXASRCTYPE  134     /* CREATE XAdatasource TYPE */
#define SQ_CRXADTSRC    135     /* CREATE XAdatasource */
#define SQ_DROPXATYPE   136     /* DROP XAdatasource  TYPE */
#define SQ_DROPXADTSRC  137     /* DROP XAdatasource */

#define SQ_SETSVPT      159     /* SAVEPOINT */
#define SQ_RELSVPT      160     /* RELEASE SAVEPOINT */
#define SQ_RBACKSVPT	161	/* ROLLBACK TO SAVEPOINT */

#define SQ_SELINTOEXT   164     /* SELECT INTO EXTERNAL  */

#define SUBC_NOT_SPECIFIED       0	/* sub clauses not specified */
#define SUBC_INSERT_VALUES       2	/* VALUES  sub clauses on INSERT stmt */

/* If you add any more sql statements don't forget that you also need to
 * make changes to sqscbinf.h, sql.msg, and sysmaster.sql to reflect the new
 * statement number.
 */
#define	SQ_MAXSTMT	SQ_SELINTOEXT

#endif /* SQLSTYPE_H_INCLUDED */
#line 202 "/home/informix/1210FC5/incl/dmi/sqlstype.h"
/* 
 * $include sqltypes;
 */
#line 57 "database.ec"

#line 57 "database.ec"
#line 1 "/home/informix/1210FC5/incl/dmi/sqltypes.h"
/****************************************************************************
 *
 * Licensed Material - Property Of IBM
 *
 * "Restricted Materials of IBM"
 *
 * IBM Informix Client SDK
 *
 * Copyright IBM Corporation 1985, 2012
 *
 *  Title:	  sqltypes.h
 *  Description:  type definition
 *
 ***************************************************************************
 */

#ifndef CCHARTYPE

#include "ifxtypes.h"

/***********************
 * ++++ CAUTION ++++
 * Any new type to be added to the following lists should not
 * have the following bit pattern (binary short):
 *
 *	xxxxx111xxxxxxxx
 *
 * where x can be either 0 or 1.
 *
 * This is due to the use of the bits as SQLNONULL, SQLHOST and SQLNETFLT
 * (see below).
 *
 * FAILURE TO DO SO WOULD RESULT IN POSSIBLE ERRORS DURING CONVERSIONS.
 *
 ***********************/

 /* C language types */

#define CCHARTYPE	100
#define CSHORTTYPE	101
#define CINTTYPE	102
#define CLONGTYPE	103
#define CFLOATTYPE	104
#define CDOUBLETYPE	105
#define CDECIMALTYPE	107
#define CFIXCHARTYPE	108
#define CSTRINGTYPE	109
#define CDATETYPE	110
#define CMONEYTYPE	111
#define CDTIMETYPE	112
#define CLOCATORTYPE    113
#define CVCHARTYPE	114
#define CINVTYPE	115
#define CFILETYPE	116
#define CINT8TYPE	117
#define CCOLLTYPE       118
#define CLVCHARTYPE     119
#define CFIXBINTYPE     120
#define CVARBINTYPE     121
#define CBOOLTYPE       122
#define CROWTYPE        123
#define CLVCHARPTRTYPE  124
#define CBIGINTTYPE     125
#define CTYPEMAX	26

#define USERCOLL(x)	((x))

#define COLLMASK        0x007F  /* mask out CTYPEDCOLL or CCLIENTCOLL */
                                /* bit set for CCOLLTYPE or CROWTYPE */
#define ISCOLLECTIONVAR(n)  (((n) & COLLMASK) == CCOLLTYPE)
#define ISROWVAR(n)         (((n) & COLLMASK) == CROWTYPE)
#define ISCOLL_OR_ROWVAR(n)   ((ISCOLLECTIONVAR(n)) || (ISROWVAR(n)))
#define CCLIENTCOLL     SQLCLIENTCOLL /* client collection bit */
#define ISCLIENTCOLLECTION(n) (ISCOLLECTIONVAR(n) && ((n) & CCLIENTCOLL))
#define ISCLIENTCOMPLEX(n)    ((ISCLIENTCOLLECTION(n)) || (ISROWVAR(n)))

/*
 * The following are for client side only. They are included here
 * because of the above related definitions.
 */
#define CTYPEDCOLL       0x0080  /* typed collection bit */
#define CTYPEDCOLLUNMASK 0xFF7F  /* unmask typed collection bit */
#define ISTYPEDCOLLECTION(n)  (ISCOLLECTIONVAR(n) && ((n) & CTYPEDCOLL))
#define ISTYPEDROW(n)         (ISROWVAR(n) && ((n) & CTYPEDCOLL))
#define ISTYPEDCOLL_OR_ROW(n)  ( (ISTYPEDCOLLECTION(n)) || (ISTYPEDROW(n)) )

/*
 * Define all possible database types
 *   include C-ISAM types here as well as in isam.h
 */

#define SQLCHAR		0
#define SQLSMINT	1
#define SQLINT		2
#define SQLFLOAT	3
#define SQLSMFLOAT	4
#define SQLDECIMAL	5
#define SQLSERIAL	6
#define SQLDATE		7
#define SQLMONEY	8
#define SQLNULL		9
#define SQLDTIME	10
#define SQLBYTES	11
#define SQLTEXT		12
#define SQLVCHAR	13
#define SQLINTERVAL	14
#define SQLNCHAR	15
#define SQLNVCHAR	16
#define SQLINT8		17
#define SQLSERIAL8	18
#define SQLSET          19
#define SQLMULTISET     20
#define SQLLIST         21
#define SQLROW          22
#define SQLCOLLECTION   23
#define SQLROWREF   	24
/*
 * Note: SQLXXX values from 25 through 39 are reserved to avoid collision
 *       with reserved PTXXX values in that same range. See p_types_t.h
 *
 * REFSER8: create tab with ref: referenced serial 8 rsam counter
 *	this is essentially a SERIAL8, but is an additional rsam counter
 *	this type only lives in the system catalogs and when read from
 *	disk is converted to SQLSERIAL8 with CD_REFSER8 set in ddcol_t
 *      ddc_flags we must distinguish from SERIAL8 to allow both
 *      counters in one tab
 */
#define SQLUDTVAR   	40
#define SQLUDTFIXED   	41
#define SQLREFSER8   	42

/* These types are used by FE, they are not real major types in BE */
#define SQLLVARCHAR     43
#define SQLSENDRECV     44
#define SQLBOOL         45
#define SQLIMPEXP       46
#define SQLIMPEXPBIN    47

/* This type is used by the UDR code to track default parameters,
   it is not a real major type in BE */
#define SQLUDRDEFAULT   48
#define SQLDBSENDRECV   49
#define SQLSRVSENDRECV  50

/* Type used by DESCRIBE INPUT stmt to indicate input parameters whose
   types cannot be determined by the server */
#define SQLUNKNOWN   	51

#define SQLINFXBIGINT   52 /* Changing to avoid collision with ODBC */
#define SQLBIGSERIAL    53

#define SQLMAXTYPES     54

#define SQLLABEL        SQLINT

#define SQLTYPE		0xFF	/* type mask		*/

#define SQLNONULL	0x0100	/* disallow nulls	*/
/* a bit to show that the value is from a host variable */
#define SQLHOST		0x0200	/* Value is from host var. */
#define SQLNETFLT	0x0400	/* float-to-decimal for networked backend */
#define SQLDISTINCT	0x0800	/* distinct bit		*/
#define SQLNAMED	0x1000	/* Named row type vs row type */
#define SQLDLVARCHAR    0x2000  /* Distinct of lvarchar */
#define SQLDBOOLEAN     0x4000  /* Distinct of boolean */
#define SQLCLIENTCOLL   0x8000  /* Collection is processed on client */

/* we are overloading SQLDBOOLEAN for use with row types */
#define SQLVARROWTYPE   0x4000  /* varlen row type */

/* We overload SQLNAMED for use with constructor type, this flag
 * distinguish constructor types from other UDT types.
 *
 * Please do not test this bit directly, use macro ISCSTTYPE() instead.
 */
#define SQLCSTTYPE	0x1000	/* constructor type flag */

#define TYPEIDMASK      (SQLTYPE | SQLDISTINCT | SQLNAMED | \
                         SQLDLVARCHAR | SQLDBOOLEAN )

#define SIZCHAR		1
#define SIZSMINT	2
#define SIZINT		4
#define SIZFLOAT	(sizeof(double))
#define SIZSMFLOAT	(sizeof(float))
#define SIZDECIMAL	17	/* decimal(32) */
#define SIZSERIAL	4
#define SIZDATE		4
#define SIZMONEY	17	/* decimal(32) */
#define SIZDTIME	7	/* decimal(12,0) */
#define SIZVCHAR	1
#define SIZINT8         (sizeof(short) + sizeof(muint) * 2)
#define SIZSERIAL8	SIZINT8
#define SIZBIGINT    8
#define SIZBIGSERIAL 8
#define SIZCOLL		sizeof (ifx_collection_t)
#define SIZSET		SIZCOLL
#define SIZMULTISET	SIZCOLL
#define SIZLIST		SIZCOLL
#define SIZROWREF	sizeof (ifx_ref_t)

#define MASKNONULL(t)	((t) & (SQLTYPE))

/*
 * As part of an sqlda structure from DESCRIBE, you can test whether a
 * column accepts or can return nulls, using the expression:
 *     ISCOLUMNULLABLE(ud->sqlvar[n].sqlflags)
 * (for sqlda structure pointer ud and column number n).
 */

#define ISCOLUMNULLABLE(t)	(((t) & (SQLNONULL)) ? 0 : 1)
#define ISSQLTYPE(t)	(MASKNONULL(t) >= SQLCHAR && MASKNONULL(t) < SQLMAXTYPES)

#define DECCOLLEN       8192    /* decimal size definition for DEC(32,0) in syscolumns */


/*
 * SQL types macros
 */
#define ISDECTYPE(t)		(MASKNONULL(t) == SQLDECIMAL || \
				 MASKNONULL(t) == SQLMONEY || \
				 MASKNONULL(t) == SQLDTIME || \
				 MASKNONULL(t) == SQLINTERVAL)
#define ISNUMERICTYPE(t)	(MASKNONULL(t) == SQLSMINT || \
				 MASKNONULL(t) == SQLINT || \
				 MASKNONULL(t) == SQLINT8 || \
				 MASKNONULL(t) == SQLFLOAT || \
				 MASKNONULL(t) == SQLSMFLOAT || \
				 MASKNONULL(t) == SQLMONEY || \
				 MASKNONULL(t) == SQLSERIAL || \
				 MASKNONULL(t) == SQLSERIAL8 || \
  				 MASKNONULL(t) == SQLDECIMAL || \
				 MASKNONULL(t) == SQLINFXBIGINT || \
				 MASKNONULL(t) == SQLBIGSERIAL)
#define ISBLOBTYPE(type)	(ISBYTESTYPE (type) || ISTEXTTYPE(type))
#define ISBYTESTYPE(type)	(MASKNONULL(type) == SQLBYTES)
#define ISTEXTTYPE(type)	(MASKNONULL(type) == SQLTEXT)
#define ISSQLHOST(t)            (((t) & SQLHOST) == SQLHOST)

#ifndef NLS
#define ISVCTYPE(t)		(MASKNONULL(t) == SQLVCHAR)
#define ISCHARTYPE(t)		(MASKNONULL(t) == SQLCHAR)
#else
#define ISVCTYPE(t)		(MASKNONULL(t) == SQLVCHAR || \
				 MASKNONULL(t) == SQLNVCHAR)
#define ISCHARTYPE(t)		(MASKNONULL(t) == SQLCHAR || \
				 MASKNONULL(t) == SQLNCHAR)
#define ISNSTRINGTYPE(t)	(MASKNONULL(t) == SQLNCHAR || \
				 MASKNONULL(t) == SQLNVCHAR)
#endif /* NLS */

#define ISSTRINGTYPE(t)		(ISVCTYPE(t) || ISCHARTYPE(t))

#define	ISUDTVARTYPE(t)		(MASKNONULL(t) == SQLUDTVAR)
#define	ISUDTFIXEDTYPE(t)	(MASKNONULL(t) == SQLUDTFIXED)
#define	ISUDTTYPE(t)		(ISUDTVARTYPE(t) || ISUDTFIXEDTYPE(t))

#define	ISCOMPLEXTYPE(t)	(ISROWTYPE(t) || ISCOLLTYPE(t))
#define	ISROWTYPE(t)		(MASKNONULL(t) == SQLROW)
#define	ISLISTTYPE(t)		(MASKNONULL(t) == SQLLIST)
#define	ISMULTISETTYPE(t)	(MASKNONULL(t) == SQLMULTISET)
#define	ISREFTYPE(t)		(MASKNONULL(t) == SQLROWREF)
#define	ISREFSER8(t)		(MASKNONULL(t) == SQLREFSER8)
#define	ISSERIAL(t)		(MASKNONULL(t) == SQLSERIAL)
#define	ISSERIAL8(t)		(MASKNONULL(t) == SQLSERIAL8)
#define ISBIGSERIAL(t)          (MASKNONULL(t) == SQLBIGSERIAL)
#define ISBIGINT(t)             (MASKNONULL(t) == SQLINFXBIGINT)
#define	ISSETTYPE(t)		(MASKNONULL(t) == SQLSET)
#define	ISCOLLECTTYPE(t)	(MASKNONULL(t) == SQLCOLLECTION)
#define ISCOLLTYPE(t)		(ISSETTYPE(t) || ISMULTISETTYPE(t) ||\
				 ISLISTTYPE(t) || ISCOLLECTTYPE(t))

#define ISSERIALTYPE(t)               (((t) & SQLTYPE) == SQLSERIAL || \
                                ((t) & SQLTYPE) == SQLSERIAL8 || \
                                ((t) & SQLTYPE) == SQLREFSER8 ||\
                                ((t) & SQLTYPE) == SQLBIGSERIAL)

#define ISDISTINCTTYPE(t)	((t) & SQLDISTINCT)
#define ISCSTTYPE(t)		(ISUDTTYPE(t) && ((t) & SQLCSTTYPE))

/* these macros are used to distinguish NLS char types and non-nls (ASCII)
 * char types
 */
#define ISNONNLSCHAR(t)		(MASKNONULL(t) == SQLCHAR || \
				 MASKNONULL(t) == SQLVCHAR)

/* these macros should be used in case statements
 */
#ifndef NLS
#define CHARCASE		SQLCHAR
#define VCHARCASE		SQLVCHAR
#else
#define CHARCASE		SQLCHAR: case SQLNCHAR
#define VCHARCASE		SQLVCHAR: case SQLNVCHAR
#endif /* NLS */

#define UDTCASE		 	SQLUDTVAR: case SQLUDTFIXED

/*
 * C types macros
 */
#define ISBLOBCTYPE(type)	(ISLOCTYPE(type) || ISFILETYPE(type))
#define ISLOCTYPE(type)		(MASKNONULL(type) == CLOCATORTYPE)
#define ISFILETYPE(type)	(MASKNONULL(type) == CFILETYPE)
#define ISLVCHARCTYPE(type)     (MASKNONULL(type) == CLVCHARTYPE)
#define ISLVCHARCPTRTYPE(type)  (MASKNONULL(type) == CLVCHARPTRTYPE)
#define ISFIXBINCTYPE(type)     (MASKNONULL(type) == CFIXBINTYPE)
#define ISVARBINCTYPE(type)     (MASKNONULL(type) == CVARBINTYPE)
#define ISBOOLCTYPE(type)       (MASKNONULL(type) == CBOOLTYPE)


#define ISOPTICALCOL(type)	(type == 'O')

#define DEFDECIMAL	9	/* default decimal(16) size */
#define DEFMONEY	9	/* default decimal(16) size */

#define SYSPUBLIC	"public"

/*
 * if an SQL type is specified, convert to default C type
 *  map C int to either short or long
 */


#define TYPEMAX	SQLMAXTYPES

extern int2 sqlctype[];

#define toctype(ctype, type) \
    { \
    if (type == CINTTYPE) \
    { \
      if (sizeof(mint) == sizeof(mlong)) \
          { ctype = type = CLONGTYPE; } \
      else if (sizeof(mint) == sizeof(int2)) \
          { ctype = type = CSHORTTYPE; } \
      else \
          { ctype = CLONGTYPE; type = CINTTYPE; } \
    } \
    else if (type >= 0 && type < TYPEMAX) \
        ctype = sqlctype[type]; \
    else \
        ctype = type; \
    }



/* Extended ID definitions for predefined UDT's */
/* These can not be changed because sqli is using
 * them.  If we change them, the client has to recompile.
 * NOTE: This order must match the definitions in boot90.sql
 */

#define XID_LVARCHAR            1
#define XID_SENDRECV            2
#define XID_IMPEXP              3
#define XID_IMPEXPBIN           4
#define XID_BOOLEAN             5
#define XID_POINTER             6
#define XID_INDEXKEYARRAY       7
#define XID_RTNPARAMTYPES	8
#define XID_SELFUNCARGS         9
#define XID_BLOB                10
#define XID_CLOB                11
#define XID_LOLIST              12
#define XID_IFX_LO_SPEC		13
#define XID_IFX_LO_STAT		14
#define XID_STAT                15
#define XID_CLIENTBINVAL        16
#define XID_UDTMODIFIERS	17
#define XID_AGGMODIFIERS	18
#define XID_UDRMODIFIERS	19
#define XID_GUID        	20
#define XID_DBSENDRECV          21
#define XID_SRVSENDRECV         22
#define XID_FUNCARG		23
#define LASTBUILTINXTDTYPE      XID_FUNCARG

/* Max size definitions for the predefined UDT's.
 * Only a few are currently defined.
 */
#define SIZINDEXKEYARRAY	1024
#define SIZLVARCHAR		2048
#define SIZRTNPARAMTYPES	4096
#define SIZSTAT                 272
#define SIZGUID			16

/* Alignment required by predefined UDT's.
 * Only a few are currently defined.  At a minimum,
 * all with alignment not 1 should be defined here
 * and used throughout the code.
 */
#define ALNINDEXKEYARRAY	4
#define ALNSTAT                 8


#define USER_XTDTYPEID_START	2048

/* These macros should be used to test lvarchar and distinct of lvarchar */

#define ISLVARCHARXTYPE(t, xid)  (ISUDTTYPE((t)) && (xid) == XID_LVARCHAR)
#define ISDISTINCTLVARCHAR(t)	((t) & SQLDLVARCHAR)
#define LIKELVARCHARXTYPE(t,xid) ((ISLVARCHARXTYPE(t,xid))||\
				   ISDISTINCTLVARCHAR(t))

#define ISSMARTBLOB(type, xid)  (ISUDTTYPE((type)) && \
				    ((xid == XID_CLOB) || (xid == XID_BLOB)))

/* These macros should be used to test boolean and distinct of boolean */
#define ISBOOLEANXTYPE(t, xid)  (ISUDTTYPE((t)) && (xid) == XID_BOOLEAN)
#define ISDISTINCTBOOLEAN(t)	(((t) & SQLDBOOLEAN) && \
				    (ISUDTTYPE(t)))
#define LIKEBOOLEANXTYPE(t,xid) ((ISBOOLEANXTYPE(t,xid))||\
				  ISDISTINCTBOOLEAN(t))

#define ISFIXLENGTHTYPE(t)	(!ISBYTESTYPE(t) && !ISTEXTTYPE(t) \
					&& !ISCOMPLEXTYPE(t) \
					&& !ISUDTVARTYPE(t) \
					&& !ISVCTYPE(t))
#endif /* CCHARTYPE */
#line 420 "/home/informix/1210FC5/incl/dmi/sqltypes.h"
/* 
 * $include sqlca;
 */
#line 58 "database.ec"

#line 58 "database.ec"
#line 1 "/home/informix/1210FC5/incl/dmi/sqlca.h"
/****************************************************************************
 *
 * Licensed Material - Property Of IBM
 *
 * "Restricted Materials of IBM"
 *
 * IBM Informix Client SDK
 * Copyright IBM Corporation 1997, 2008. All rights reserved.
 *
 *  Title:       sqlca.h
 *  Description: SQL Control Area
 *
 ***************************************************************************
 */

#ifndef SQLCA_INCL
#define SQLCA_INCL

#include "ifxtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sqlca_s
    {
    int4 sqlcode;
#ifdef DB2CLI
    char sqlerrm[600]; /* error message parameters */
#else /* DB2CLI */
    char sqlerrm[72]; /* error message parameters */
#endif /* DB2CLI */
    char sqlerrp[8];
    int4 sqlerrd[6];
		    /* 0 - estimated number of rows returned */
		    /* 1 - serial value after insert or  ISAM error code */
		    /* 2 - number of rows processed */
		    /* 3 - estimated cost */
		    /* 4 - offset of the error into the SQL statement */
		    /* 5 - rowid after insert  */
#ifdef _FGL_
    char sqlawarn[8];
#else
    struct sqlcaw_s
	{
	char sqlwarn0; /* = W if any of sqlwarn[1-7] = W */
	char sqlwarn1; /* = W if any truncation occurred or
				database has transactions or
			        no privileges revoked */
	char sqlwarn2; /* = W if a null value returned or
				ANSI database */
	char sqlwarn3; /* = W if no. in select list != no. in into list or
				turbo backend or no privileges granted */
	char sqlwarn4; /* = W if no where clause on prepared update, delete or
				incompatible float format */
	char sqlwarn5; /* = W if non-ANSI statement */
	char sqlwarn6; /* = W if server is in data replication secondary mode */
	char sqlwarn7; /* = W if database locale is different from proc_locale
                          = W if backend XPS and if explain avoid_execute is set
                              (for select, insert, delete and update only)
			*/
	} sqlwarn;
#endif
    } ifx_sqlca_t;

/* NOTE: 4gl assumes that the sqlwarn structure can be defined as
 *	sqlawarn -- an 8 character string, because single-char
 *	variables are not recognized in 4gl.
 *
 * If this structure should change, the code generated by 4gl compiler
 *	must also change
 */

#define SQLNOTFOUND 100

#ifndef IFX_THREAD
#ifdef DB2CLI
#define sqlca ifmxsqlca
extern struct sqlca_s sqlca;
#else /* DB2CLI */
extern struct sqlca_s sqlca;
#endif /* DB2CLI */

#ifndef DRDAHELP
extern int4 SQLCODE;
#endif

extern char SQLSTATE[];
#else /* IFX_THREAD */
extern int4 * ifx_sqlcode(void);
extern struct sqlca_s * ifx_sqlca(void);
/* ifx_sqlstate() declared in sqlhdr.h */
#define SQLCODE (*(ifx_sqlcode()))
#define SQLSTATE ((char *)(ifx_sqlstate()))
#define sqlca (*(ifx_sqlca()))
#endif /* IFX_THREAD */

#ifdef __cplusplus
}
#endif

#endif /* SQLCA_INCL */

#line 103 "/home/informix/1210FC5/incl/dmi/sqlca.h"
#line 59 "database.ec"

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
/*
 *     $char  database[(128*2)+2];
 */
#line 76 "database.ec"
  char database[258];
/*
 *     $db_prep *p;
 */
#line 77 "database.ec"
db_prep *p;
/*
 *     $char  *passwd = g->password;
 */
#line 78 "database.ec"
  char *passwd = g->password;
/*
 *     $char  *user = g->user;
 */
#line 79 "database.ec"
  char *user = g->user;

    p = t->connection->prepared;

    sprintf (database, "%s@%s", g->database,
	     g->server_list[node].name);

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 86 "database.ec"

    if (user)
/*
 * 	$CONNECT TO $database AS $p->connection_id
 * 	    USER $user USING $passwd;
 */
#line 89 "database.ec"
  {
#line 90 "database.ec"
  ifx_conn_t      *_sqiconn;
  _sqiconn = (ifx_conn_t *)ifx_alloc_conn_user(user, passwd);
  sqli_connect_open(ESQLINTVERSION, 0, database, p->connection_id, _sqiconn, 0);
  ifx_free_conn_user(&_sqiconn);
#line 90 "database.ec"
  { if (SQLCODE < 0) SQL_Check(); }
#line 90 "database.ec"
}
    else
/*
 * 	$CONNECT TO $database AS $p->connection_id;
 */
#line 92 "database.ec"
{
#line 92 "database.ec"
sqli_connect_open(ESQLINTVERSION, 0, database, p->connection_id, (ifx_conn_t *)0, 0);
#line 92 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 92 "database.ec"
}

    return (sqlca.sqlcode == 0 ? 1 : 0);
}

int connect_without_error (db_thread *t, int node)
{
    global  *g = t->globals;
/*
 *     $char  database[(128*2)+2];
 */
#line 100 "database.ec"
  char database[258];
/*
 *     $db_prep *p;
 */
#line 101 "database.ec"
db_prep *p;
/*
 *     $char  *passwd = g->password;
 */
#line 102 "database.ec"
  char *passwd = g->password;
/*
 *     $char  *user = g->user;
 */
#line 103 "database.ec"
  char *user = g->user;

    p = t->connection->prepared;

    sprintf (database, "%s@%s", g->database,
	     g->server_list[node].name);

/*
 *     $WHENEVER ERROR CONTINUE;
 */
#line 110 "database.ec"

    if (g->user)
/*
 * 	$CONNECT TO $database AS $p->connection_id
 * 	    USER $user USING $passwd;
 */
#line 113 "database.ec"
{
#line 114 "database.ec"
ifx_conn_t      *_sqiconn;
_sqiconn = (ifx_conn_t *)ifx_alloc_conn_user(user, passwd);
sqli_connect_open(ESQLINTVERSION, 0, database, p->connection_id, _sqiconn, 0);
ifx_free_conn_user(&_sqiconn);
#line 114 "database.ec"
}
    else
/*
 * 	$CONNECT TO $database AS $p->connection_id;
 */
#line 116 "database.ec"
{
#line 116 "database.ec"
sqli_connect_open(ESQLINTVERSION, 0, database, p->connection_id, (ifx_conn_t *)0, 0);
#line 116 "database.ec"
}

    return (sqlca.sqlcode == 0 ? 1 : 0);
}

/*
 * $WHENEVER ERROR CALL SQL_Check;
 */
#line 121 "database.ec"

int connect_to (db_thread *t)
{
    int        connected = 0;
    db_conn   *c;
    global    *g;
/*
 *     $char      wait_stmt[32];
 */
#line 128 "database.ec"
  char wait_stmt[32];

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

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 145 "database.ec"

    if (g->isolation)
    {
	if (strcmp (g->isolation, "RR") == 0)
	{
/*
 * 	    $SET ISOLATION REPEATABLE READ;
 */
#line 151 "database.ec"
{
#line 151 "database.ec"
static const char *sqlcmdtxt[] =
#line 151 "database.ec"
{
#line 151 "database.ec"
"SET ISOLATION REPEATABLE READ",
0
};
#line 151 "database.ec"
ifx_statement_t _SQ0;
#line 151 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 151 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 151 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 151 "database.ec"
}
	}
	else if (strcmp (g->isolation, "CR") == 0)
	{
/*
 * 	    $SET ISOLATION COMMITTED READ;
 */
#line 155 "database.ec"
{
#line 155 "database.ec"
static const char *sqlcmdtxt[] =
#line 155 "database.ec"
{
#line 155 "database.ec"
"SET ISOLATION COMMITTED READ",
0
};
#line 155 "database.ec"
ifx_statement_t _SQ0;
#line 155 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 155 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 155 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 155 "database.ec"
}
	}
	else if (strcmp (g->isolation, "CS") == 0)
	{
/*
 * 	    $SET ISOLATION CURSOR STABILITY;
 */
#line 159 "database.ec"
{
#line 159 "database.ec"
static const char *sqlcmdtxt[] =
#line 159 "database.ec"
{
#line 159 "database.ec"
"SET ISOLATION CURSOR STABILITY",
0
};
#line 159 "database.ec"
ifx_statement_t _SQ0;
#line 159 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 159 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 159 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 159 "database.ec"
}
	}
	else if (strcmp (g->isolation, "DR") == 0)
	{
/*
 * 	    $SET ISOLATION DIRTY READ;
 */
#line 163 "database.ec"
{
#line 163 "database.ec"
static const char *sqlcmdtxt[] =
#line 163 "database.ec"
{
#line 163 "database.ec"
"SET ISOLATION DIRTY READ",
0
};
#line 163 "database.ec"
ifx_statement_t _SQ0;
#line 163 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 163 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 163 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 163 "database.ec"
}
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

/*
 *     $EXECUTE IMMEDIATE $wait_stmt;
 */
#line 180 "database.ec"
{
#line 180 "database.ec"
sqli_exec_immed(wait_stmt);
#line 180 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 180 "database.ec"
}

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
/*
 *     $struct db_prep_s    *p;
 */
#line 244 "database.ec"
struct db_prep_s *p;
/*
 *     $char                 version[128] = { '\0' };
 */
#line 245 "database.ec"
  char version[128] = { '\0' };

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
	
/*
 * 	$EXECUTE FUNCTION TimeseriesRelease () INTO $version;
 */
#line 264 "database.ec"
{
#line 264 "database.ec"
static const char *sqlcmdtxt[] =
#line 264 "database.ec"
{
#line 264 "database.ec"
"EXECUTE FUNCTION TimeseriesRelease ( )",
0
};
#line 264 "database.ec"
ifx_cursor_t _SQ0;
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
sqli_mt_obind(100, version, 128, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
sqli_proc_exec_2(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, 1, (ifx_sqlvar_t *)_SD0->sqlvar, 0, 0, 1);
#line 264 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 264 "database.ec"
}

	if (t->globals->verbose)
	    printf ("%s: Server %s\n", t->name, version);
	
/*
 * 	$PREPARE $p->init_table_id
 * 	    FROM "EXECUTE FUNCTION TSL_Init (?,?,?,?,?,?,?,?)";
 */
#line 269 "database.ec"
{
#line 270 "database.ec"
sqli_prep(ESQLINTVERSION, p->init_table_id, "EXECUTE FUNCTION TSL_Init (?,?,?,?,?,?,?,?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 270 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 270 "database.ec"
}

/*
 * 	$PREPARE $p->release_id
 * 	    FROM "EXECUTE FUNCTION TSL_SessionClose (?)";
 */
#line 272 "database.ec"
{
#line 273 "database.ec"
sqli_prep(ESQLINTVERSION, p->release_id, "EXECUTE FUNCTION TSL_SessionClose (?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 273 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 273 "database.ec"
}
	
/*
 * 	$PREPARE $p->shutdown_id
 * 	    FROM "EXECUTE PROCEDURE TSL_Shutdown (?)";
 */
#line 275 "database.ec"
{
#line 276 "database.ec"
sqli_prep(ESQLINTVERSION, p->shutdown_id, "EXECUTE PROCEDURE TSL_Shutdown (?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 276 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 276 "database.ec"
}

	break;

    case T_LOADER:

/*
 *         $EXECUTE PROCEDURE Ifx_Allow_Newline('t');
 */
#line 282 "database.ec"
{
#line 282 "database.ec"
static const char *sqlcmdtxt[] =
#line 282 "database.ec"
{
#line 282 "database.ec"
"EXECUTE PROCEDURE Ifx_Allow_Newline ( 't' )",
0
};
#line 282 "database.ec"
ifx_statement_t _SQ0;
#line 282 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 282 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 282 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 282 "database.ec"
}
	
/*
 * 	$PREPARE $p->attach_id
 * 	    FROM "EXECUTE FUNCTION TSL_Attach (?,?,?)";
 */
#line 284 "database.ec"
{
#line 285 "database.ec"
sqli_prep(ESQLINTVERSION, p->attach_id, "EXECUTE FUNCTION TSL_Attach (?,?,?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 285 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 285 "database.ec"
}
	
/*
 * 	$PREPARE $p->put_data_id
 * 	    FROM "EXECUTE FUNCTION TSL_Put (?,?)";
 */
#line 287 "database.ec"
{
#line 288 "database.ec"
sqli_prep(ESQLINTVERSION, p->put_data_id, "EXECUTE FUNCTION TSL_Put (?,?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 288 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 288 "database.ec"
}
	
/*
 * 	$PREPARE $p->put_sql_id
 * 	    FROM "EXECUTE FUNCTION TSL_PutSQL (?,?)";
 */
#line 290 "database.ec"
{
#line 291 "database.ec"
sqli_prep(ESQLINTVERSION, p->put_sql_id, "EXECUTE FUNCTION TSL_PutSQL (?,?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 291 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 291 "database.ec"
}
	
/*
 * 	$PREPARE $p->release_id
 * 	    FROM "EXECUTE FUNCTION TSL_SessionClose (?)";
 */
#line 293 "database.ec"
{
#line 294 "database.ec"
sqli_prep(ESQLINTVERSION, p->release_id, "EXECUTE FUNCTION TSL_SessionClose (?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 294 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 294 "database.ec"
}
	
/*
 * 	$PREPARE $p->lock_container_id
 * 	    FROM "EXECUTE PROCEDURE TSContainerLock (?,?)";
 */
#line 296 "database.ec"
{
#line 297 "database.ec"
sqli_prep(ESQLINTVERSION, p->lock_container_id, "EXECUTE PROCEDURE TSContainerLock (?,?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 297 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 297 "database.ec"
}
	
/*
 * 	$PREPARE $p->flush_data_id
 * 	    FROM "EXECUTE FUNCTION TSL_FlushAll (?,?)";
 */
#line 299 "database.ec"
{
#line 300 "database.ec"
sqli_prep(ESQLINTVERSION, p->flush_data_id, "EXECUTE FUNCTION TSL_FlushAll (?,?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 300 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 300 "database.ec"
}
	
/*
 * 	$PREPARE $p->commit_data_id
 * 	    FROM "EXECUTE FUNCTION TSL_Commit (?,?,?)";
 */
#line 302 "database.ec"
{
#line 303 "database.ec"
sqli_prep(ESQLINTVERSION, p->commit_data_id, "EXECUTE FUNCTION TSL_Commit (?,?,?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 303 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 303 "database.ec"
}

/*
 * 	$PREPARE $p->info_id
 * 	    FROM "SELECT info.* FROM TABLE (TSL_FlushInfo (?::LVARCHAR)) AS t (info)";
 */
#line 305 "database.ec"
{
#line 306 "database.ec"
sqli_prep(ESQLINTVERSION, p->info_id, "SELECT info.* FROM TABLE (TSL_FlushInfo (?::LVARCHAR)) AS t (info)",(ifx_literal_t *)0, (ifx_namelist_t *)0, 2, 0, 0 ); 
#line 306 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 306 "database.ec"
}

	break;

    case T_STORAGE:
    case T_REWRITE:

/*
 * 	$PREPARE $p->storage_id
 * 	    FROM "INSERT INTO ts_storage VALUES (?, ?, ?)";
 */
#line 313 "database.ec"
{
#line 314 "database.ec"
sqli_prep(ESQLINTVERSION, p->storage_id, "INSERT INTO ts_storage VALUES (?, ?, ?)",(ifx_literal_t *)0, (ifx_namelist_t *)0, 6, 0, 0 ); 
#line 314 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 314 "database.ec"
}

/*
 * 	$PREPARE $p->next_device_id
 * 	    FROM "EXECUTE FUNCTION next_device ()";
 */
#line 316 "database.ec"
{
#line 317 "database.ec"
sqli_prep(ESQLINTVERSION, p->next_device_id, "EXECUTE FUNCTION next_device ()",(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 317 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 317 "database.ec"
}
	
	break;
	
    default:
	break;


    }

  return c->connected;
}

int table_attach (db_thread *t)
{
/*
 *     $varchar    *s_tab_name, *s_ts_col,
 *  	         s_reject[128*8] = "", s_handle[(128*2)+1];
 */
#line 332 "database.ec"
  char *s_tab_name, *s_ts_col, s_reject[1024] = "", s_handle[257];
/*
 *     $db_prep    *p = t->connection->prepared;
 */
#line 334 "database.ec"
db_prep *p = t->connection->prepared;
/*
 *     $short       rejectind = 0;
 */
#line 335 "database.ec"
short rejectind = 0;

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

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 366 "database.ec"

/*
 *     $EXECUTE $p->attach_id INTO $s_handle USING
 * 	$s_tab_name, $s_ts_col, $s_reject:rejectind;
 */
#line 368 "database.ec"
{
#line 369 "database.ec"
ifx_sqlda_t *_SD0;
ifx_sqlda_t *_SD1;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,3);
_dummy_SD0 (_SD0);
_SD1 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
sqli_mt_ibind(114, s_tab_name, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, s_ts_col, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, s_reject, 1024, 101, (char *) &rejectind, sizeof(rejectind), 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(114, s_handle, 257, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->attach_id, 257), _SD0, (char *)0, (struct value *)0, _SD1, (char *)0, (struct value *)0, 0);
#line 369 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 369 "database.ec"
}

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
/*
 *     $varchar *s_tab_name, *s_ts_col,
 * 	      s_logfile[1024] = "", s_rejectfile[1024] = "",
 * 	     *s_where, *s_dt_format, s_handle[(128*2)+1];
 */
#line 390 "database.ec"
  char *s_tab_name, *s_ts_col, s_logfile[1024] = "", s_rejectfile[1024] = "", *s_where, *s_dt_format, s_handle[257];
/*
 *     $int      i_logtype, i_loglevel;
 */
#line 393 "database.ec"
int i_logtype, i_loglevel;
/*
 *     $db_prep *p = t->connection->prepared;
 */
#line 394 "database.ec"
db_prep *p = t->connection->prepared;
/*
 *     $short    whereind = 0, logfileind = 0, rejectfileind = 0, dt_formatind = 0;
 */
#line 395 "database.ec"
short whereind = 0, logfileind = 0, rejectfileind = 0, dt_formatind = 0;

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

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 454 "database.ec"

/*
 *     $EXECUTE $p->init_table_id INTO $s_handle USING
 * 	$s_tab_name, $s_ts_col, $i_logtype, $i_loglevel, $s_logfile:logfileind,
 * 	$s_dt_format:dt_formatind, $s_rejectfile:rejectfileind,
 * 	$s_where:whereind;
 */
#line 456 "database.ec"
{
#line 459 "database.ec"
ifx_sqlda_t *_SD0;
ifx_sqlda_t *_SD1;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,8);
_dummy_SD0 (_SD0);
_SD1 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
sqli_mt_ibind(114, s_tab_name, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, s_ts_col, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(102, (char *) &i_logtype, sizeof(i_logtype), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(102, (char *) &i_loglevel, sizeof(i_loglevel), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, s_logfile, 1024, 101, (char *) &logfileind, sizeof(logfileind), 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, s_dt_format, 0, 101, (char *) &dt_formatind, sizeof(dt_formatind), 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, s_rejectfile, 1024, 101, (char *) &rejectfileind, sizeof(rejectfileind), 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, s_where, 0, 101, (char *) &whereind, sizeof(whereind), 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(114, s_handle, 257, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->init_table_id, 257), _SD0, (char *)0, (struct value *)0, _SD1, (char *)0, (struct value *)0, 0);
#line 459 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 459 "database.ec"
}

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
/*
 *     $char      sql_stmt[BUF_SIZE];
 */
#line 480 "database.ec"
  char sql_stmt[BUF_SIZE];
    global    *g = t->globals;
    node      *n;
    int        i;

    /* Get the number of cpus as a guide to how many queues to start */
    g->total_queues = 0;

    for (i = 0; i < g->nservers; i++)
    {
/*
 * 	$int ncpus;
 */
#line 490 "database.ec"
int ncpus;

	n = &(g->server_list[i]);
	
	sprintf (sql_stmt, "SELECT count(*) FROM sysmaster@%s:sysvpprof where class = 'cpu'", n->name);

/*
 * 	$PREPARE cpus_id FROM $sql_stmt;
 */
#line 496 "database.ec"
{
#line 496 "database.ec"
sqli_prep(ESQLINTVERSION, _Cn1, sql_stmt,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 496 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 496 "database.ec"
}

/*
 * 	$EXECUTE cpus_id INTO $ncpus;
 */
#line 498 "database.ec"
{
#line 498 "database.ec"
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
sqli_mt_obind(102, (char *) &ncpus, sizeof(ncpus), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn1, 769), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, _SD0, (char *)0, (struct value *)0, 0);
#line 498 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 498 "database.ec"
}

/*
 * 	$FREE cpus_id;
 */
#line 500 "database.ec"
{
#line 500 "database.ec"
  sqli_mt_free(_Cn1);
#line 500 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 500 "database.ec"
}

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
/*
 *     $varchar nodename[128+1];
 */
#line 559 "database.ec"
  char nodename[129];
/*
 *     $char    sql_stmt[BUF_SIZE];
 */
#line 560 "database.ec"
  char sql_stmt[BUF_SIZE];
/*
 *     $int     nodeid;
 */
#line 561 "database.ec"
int nodeid;
/*
 *     $int     nrows;
 */
#line 562 "database.ec"
int nrows;
    node    *found_nodes = NULL;
    int      nodes, i, total_nrows;
    char    *tabname;
    global  *g;

    g = t->globals;

    /* Get server info first - nrows here is count of metadevice rows local to the node */
    found_nodes = (node *) calloc (255, sizeof (node));

    if (g->meta_table)
	tabname = g->meta_table;
    else
	tabname = g->table;

    sprintf (sql_stmt, "SELECT count(*), ifx_node_id(), TRIM(ifx_node_name()) FROM %s GRID ALL '%s' GROUP BY 2 ORDER BY 2",
	     tabname, g->grid);

/*
 *     $PREPARE get_nodes_id FROM $sql_stmt;
 */
#line 581 "database.ec"
{
#line 581 "database.ec"
sqli_prep(ESQLINTVERSION, _Cn2, sql_stmt,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 581 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 581 "database.ec"
}

/*
 *     $DECLARE get_nodes CURSOR FOR get_nodes_id;
 */
#line 583 "database.ec"
{
#line 583 "database.ec"
sqli_curs_decl_dynm(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn3, 512), _Cn3, sqli_curs_locate(ESQLINTVERSION, _Cn2, 513), 0, 0);
#line 583 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 583 "database.ec"
}

/*
 *     $OPEN get_nodes;
 */
#line 585 "database.ec"
{
#line 585 "database.ec"
sqli_curs_open(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn3, 768), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0, 0);
#line 585 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 585 "database.ec"
}

    total_nrows = nodes = 0;
    while (sqlca.sqlcode == 0)
    {
/*
 * 	$FETCH get_nodes INTO $nrows, $nodeid, $nodename;
 */
#line 590 "database.ec"
{
#line 590 "database.ec"
ifx_sqlda_t *_SD0;
static _FetchSpec _FS1 = { 0, 1, 0 };
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,3);
_dummy_SD0 (_SD0);
sqli_mt_obind(102, (char *) &nrows, sizeof(nrows), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &nodeid, sizeof(nodeid), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(114, nodename, 129, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_curs_fetch(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn3, 768), (ifx_sqlda_t *)0, _SD0, (char *)0, &_FS1);
#line 590 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 590 "database.ec"
}
	
	if (sqlca.sqlcode == 0)
	{
	    found_nodes[nodes].name = strdup (nodename);
	    found_nodes[nodes].id = nodeid;
	    found_nodes[nodes].dev_nrows = nrows;
	    nodes++;
	    total_nrows += nrows;
	}
    }	
/*
 *     $CLOSE get_nodes;
 */
#line 601 "database.ec"
{
#line 601 "database.ec"
sqli_curs_close(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn3, 768));
#line 601 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 601 "database.ec"
}

/*
 *     $FREE get_nodes;
 */
#line 603 "database.ec"
{
#line 603 "database.ec"
  sqli_mt_free(_Cn3);
#line 603 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 603 "database.ec"
}
/*
 *     $FREE get_nodes_id;
 */
#line 604 "database.ec"
{
#line 604 "database.ec"
  sqli_mt_free(_Cn2);
#line 604 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 604 "database.ec"
}

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

/*
 *     $PREPARE get_nrows_id FROM $sql_stmt;
 */
#line 628 "database.ec"
{
#line 628 "database.ec"
sqli_prep(ESQLINTVERSION, _Cn4, sql_stmt,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 628 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 628 "database.ec"
}

/*
 *     $DECLARE get_nrows CURSOR FOR get_nrows_id;
 */
#line 630 "database.ec"
{
#line 630 "database.ec"
sqli_curs_decl_dynm(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn5, 512), _Cn5, sqli_curs_locate(ESQLINTVERSION, _Cn4, 513), 0, 0);
#line 630 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 630 "database.ec"
}

/*
 *     $OPEN get_nrows;
 */
#line 632 "database.ec"
{
#line 632 "database.ec"
sqli_curs_open(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn5, 768), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0, 0);
#line 632 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 632 "database.ec"
}

    while (sqlca.sqlcode == 0)
    {
/*
 * 	$FETCH get_nrows INTO $nrows, $nodeid;
 */
#line 636 "database.ec"
{
#line 636 "database.ec"
ifx_sqlda_t *_SD0;
static _FetchSpec _FS1 = { 0, 1, 0 };
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,2);
_dummy_SD0 (_SD0);
sqli_mt_obind(102, (char *) &nrows, sizeof(nrows), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &nodeid, sizeof(nodeid), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_curs_fetch(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn5, 768), (ifx_sqlda_t *)0, _SD0, (char *)0, &_FS1);
#line 636 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 636 "database.ec"
}
	
	if (sqlca.sqlcode == 0)
	{
	    int f;

	    if ((f = find_server (t, nodeid)) != -1)
		g->server_list[f].nrows = nrows;
	}
    }

/*
 *     $CLOSE get_nrows;
 */
#line 647 "database.ec"
{
#line 647 "database.ec"
sqli_curs_close(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn5, 768));
#line 647 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 647 "database.ec"
}

/*
 *     $FREE get_nrows;
 */
#line 649 "database.ec"
{
#line 649 "database.ec"
  sqli_mt_free(_Cn5);
#line 649 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 649 "database.ec"
}
/*
 *     $FREE get_nrows_id;
 */
#line 650 "database.ec"
{
#line 650 "database.ec"
  sqli_mt_free(_Cn4);
#line 650 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 650 "database.ec"
}

    g->total_nrows = total_nrows;

    return nodes;
}

int db_put_data (db_thread *t,
/*
 * 	      EXEC SQL BEGIN DECLARE SECTION;
 */
#line 658 "database.ec"
#line 659 "database.ec"
  char *data
/*
 * 	      EXEC SQL END DECLARE SECTION;
 */
#line 660 "database.ec"

    )
{
/*
 *     $int      rval = -1;
 */
#line 663 "database.ec"
int rval = -1;
/*
 *     $varchar *s_handle = t->globals->server_list[t->connection->current_server].handle;
 */
#line 664 "database.ec"
  char *s_handle = t->globals->server_list[t->connection->current_server].handle;
/*
 *     $db_prep *p = t->connection->prepared;
 */
#line 665 "database.ec"
db_prep *p = t->connection->prepared;

    struct timeval start_tv;

    gettimeofday (&start_tv, NULL);
	
/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 671 "database.ec"

    if (data && *data != '\0')
    {
	if (t->nput == -1 || t->globals->verbose > 1)
	{
	    printf ("%s: handle <%s>, data[%ld],                       \n%.256s\n",
		    t->name, s_handle, strlen (data), data);
	    t->nput = 0;
	}
/*
 * 	$EXECUTE $p->put_data_id INTO $rval USING $s_handle, $data;
 */
#line 681 "database.ec"
{
#line 681 "database.ec"
ifx_sqlda_t *_SD0;
ifx_sqlda_t *_SD1;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,2);
_dummy_SD0 (_SD0);
_SD1 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
sqli_mt_ibind(114, s_handle, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, data, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &rval, sizeof(rval), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->put_data_id, 257), _SD0, (char *)0, (struct value *)0, _SD1, (char *)0, (struct value *)0, 0);
#line 681 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 681 "database.ec"
}
    }

    gettimeofday (&(t->last_put_tv), NULL);

    timersub (&(t->last_put_tv), &start_tv, &(t->put_elapsed));
    timerinc (&(t->total_put_elapsed), &(t->put_elapsed));

    return rval;
}

int lock_container_sql (db_thread *t,
/*
 * 			EXEC SQL BEGIN DECLARE SECTION;
 */
#line 693 "database.ec"
#line 694 "database.ec"
  char *container,
int lock
/*
 * 			EXEC SQL END DECLARE SECTION;
 */
#line 696 "database.ec"

    )
{
/*
 *   $db_prep    *p = t->connection->prepared;
 */
#line 699 "database.ec"
db_prep *p = t->connection->prepared;
  int          i = 0;

/*
 *   $WHENEVER ERROR CONTINUE;
 */
#line 702 "database.ec"

  if (*container)
  {
      for (i = 0; i < 100; i++)
      {
/*
 * 	  $EXECUTE $p->lock_container_id USING $container, $lock;
 */
#line 708 "database.ec"
{
#line 708 "database.ec"
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,2);
_dummy_SD0 (_SD0);
sqli_mt_ibind(114, container, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(102, (char *) &lock, sizeof(lock), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->lock_container_id, 257), _SD0, (char *)0, (struct value *)0, (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0);
#line 708 "database.ec"
}
	  if (sqlca.sqlcode == 0)
	      break;
      }
  }
  if (t->globals->verbose && i > 0)
      printf ("%s: Took %d attempts to lock container (sqlca.sqlcode=%d)\n",
	      t->name, i, sqlca.sqlcode);

/*
 *   $WHENEVER ERROR CALL SQL_Check;
 */
#line 717 "database.ec"

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
/*
 *     $int     nrows;
 */
#line 774 "database.ec"
int nrows;
/*
 *     $varchar cname[128+1];
 */
#line 775 "database.ec"
  char cname[129];
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

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 789 "database.ec"
	
/*
 *     $SELECT count(*) INTO $nrows FROM tscontainertable;
 */
#line 791 "database.ec"
{
#line 791 "database.ec"
static const char *sqlcmdtxt[] =
#line 791 "database.ec"
{
#line 791 "database.ec"
"SELECT count ( * ) FROM tscontainertable",
0
};
#line 791 "database.ec"
ifx_cursor_t _SQ0;
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
sqli_mt_obind(102, (char *) &nrows, sizeof(nrows), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 791 "database.ec"
sqli_slct(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, 1, (ifx_sqlvar_t *)_SD0->sqlvar, 0, (ifx_literal_t *)0, (ifx_namelist_t *)0, 0);
#line 791 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 791 "database.ec"
}

    /* Allow for growth */
    if (nrows == 0)
	n->container_table_size = MIN_CONTAINER_TABLE * 2;
    else
	n->container_table_size = nrows + MIN_CONTAINER_TABLE;

    n->container_list = (container **) malloc (nrows * sizeof (container *));
    n->container_hash_total = 0;

    hash_create (&n->container_hash, n->container_table_size);
	
/*
 *     $DECLARE get_container CURSOR FOR
 * 	SELECT TRIM(name) FROM tscontainertable;
 */
#line 804 "database.ec"
{
#line 805 "database.ec"
static const char *sqlcmdtxt[] =
#line 805 "database.ec"
{
#line 805 "database.ec"
"SELECT TRIM ( name ) FROM tscontainertable",
0
};
#line 805 "database.ec"
#line 805 "database.ec"
sqli_mt_dclcur(ESQLINTVERSION, _Cn6, sqlcmdtxt, 0, 0, 0, (ifx_literal_t *)0, (ifx_namelist_t *)0, 2, 0, 0);
#line 805 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 805 "database.ec"
}
	
/*
 *     $OPEN get_container;
 */
#line 807 "database.ec"
{
#line 807 "database.ec"
sqli_curs_open(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn6, 768), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0, 0);
#line 807 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 807 "database.ec"
}

    while (sqlca.sqlcode == 0)
    {
/*
 * 	$FETCH get_container INTO $cname;
 */
#line 811 "database.ec"
{
#line 811 "database.ec"
ifx_sqlda_t *_SD0;
static _FetchSpec _FS1 = { 0, 1, 0 };
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
sqli_mt_obind(114, cname, 129, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_curs_fetch(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn6, 768), (ifx_sqlda_t *)0, _SD0, (char *)0, &_FS1);
#line 811 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 811 "database.ec"
}
	
	if (sqlca.sqlcode == 0)
	    store_container (cname, n);
    }

/*
 *     $CLOSE get_container;
 */
#line 817 "database.ec"
{
#line 817 "database.ec"
sqli_curs_close(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn6, 768));
#line 817 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 817 "database.ec"
}

/*
 *     $FREE get_container;
 */
#line 819 "database.ec"
{
#line 819 "database.ec"
  sqli_mt_free(_Cn6);
#line 819 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 819 "database.ec"
}

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
/*
 *     $varchar  all_cols [((128+8)*32)+1];
 */
#line 841 "database.ec"
  char all_cols[4353];
/*
 *     $int      col_len;
 */
#line 842 "database.ec"
int col_len;
/*
 *     $varchar *tabname;
 */
#line 843 "database.ec"
  char *tabname;
/*
 *     $varchar  col_pk_name[128+1];
 */
#line 844 "database.ec"
  char col_pk_name[129];
/*
 *     $varchar  col_name[128+1];
 */
#line 845 "database.ec"
  char col_name[129];

    global   *g;

    g = t->globals;

    if (g->meta_table)
	tabname = g->meta_table;
    else
	tabname = g->table;

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 856 "database.ec"
	
/*
 *     $DECLARE get_pk_info CURSOR FOR
 * 	SELECT 'TRIM('||t.tabname||'.'||TRIM(c.colname)||'::lvarchar)',
 * 	TRIM(c.colname),
 * 	schema_precision (c.coltype, c.extended_id, c.collength)
 * 	FROM systables t, syscolumns c,
 * 	(SELECT ikeyextractcolno(i.indexkeys, k.counter::INTEGER)
 * 	 FROM systables t, sysconstraints p, sysindices i,
 * 	 TABLE(MULTISET{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15})
 * 	 AS k(counter)
 * 	 WHERE t.tabname = $tabname AND t.tabid = p.tabid
 * 	 AND t.tabid=i.tabid AND p.idxname = i.idxname AND p.constrtype = 'P')
 * 	AS kp(colno)
 * 	WHERE t.tabname = $tabname AND t.tabid=c.tabid AND c.colno = kp.colno
 * 	ORDER BY c.colno;
 */
#line 858 "database.ec"
{
#line 871 "database.ec"
static const char *sqlcmdtxt[] =
#line 871 "database.ec"
{
#line 871 "database.ec"
"SELECT 'TRIM(' || t . tabname || '.' || TRIM ( c . colname ) || '::lvarchar)' , TRIM ( c . colname ) , schema_precision ( c . coltype , c . extended_id , c . collength ) FROM systables t , syscolumns c , ( SELECT ikeyextractcolno ( i . indexkeys , k . counter :: INTEGER ) FROM systables t , sysconstraints p , sysindices i , TABLE ( multiset {  0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10 , 11 , 12 , 13 , 14 , 15 } ) AS k ( counter ) WHERE t . tabname = ? AND t . tabid = p . tabid AND t . tabid = i . tabid AND p . idxname = i . idxname AND p . constrtype = 'P' ) AS kp ( colno ) WHERE t . tabname = ? AND t . tabid = c . tabid AND c . colno = kp . colno ORDER BY c . colno",
0
};
#line 871 "database.ec"
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,2);
_dummy_SD0 (_SD0);
sqli_mt_ibind(114, tabname, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, tabname, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
#line 871 "database.ec"
sqli_mt_dclcur(ESQLINTVERSION, _Cn7, sqlcmdtxt, 2, 0, 0, (ifx_literal_t *)0, (ifx_namelist_t *)0, 2, 0, 0);
#line 871 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 871 "database.ec"
}

/*
 *     $OPEN get_pk_info;
 */
#line 873 "database.ec"
{
#line 873 "database.ec"
sqli_curs_open(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn7, 768), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0, 0);
#line 873 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 873 "database.ec"
}

    all_cols[0] = '\0';
    g->pk_column_count = 0;
    g->pk_column_len = 0;
    while (sqlca.sqlcode == 0)
    {
/*
 * 	$FETCH get_pk_info INTO $col_pk_name, $col_name, $col_len;
 */
#line 880 "database.ec"
{
#line 880 "database.ec"
ifx_sqlda_t *_SD0;
static _FetchSpec _FS1 = { 0, 1, 0 };
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,3);
_dummy_SD0 (_SD0);
sqli_mt_obind(114, col_pk_name, 129, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(114, col_name, 129, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &col_len, sizeof(col_len), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_curs_fetch(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn7, 768), (ifx_sqlda_t *)0, _SD0, (char *)0, &_FS1);
#line 880 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 880 "database.ec"
}
	
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
/*
 *     $CLOSE get_pk_info;
 */
#line 892 "database.ec"
{
#line 892 "database.ec"
sqli_curs_close(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn7, 768));
#line 892 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 892 "database.ec"
}

/*
 *     $FREE get_pk_info;
 */
#line 894 "database.ec"
{
#line 894 "database.ec"
  sqli_mt_free(_Cn7);
#line 894 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 894 "database.ec"
}

    g->pk_columns = strdup (all_cols);

    if (g->verbose > 1)
	printf ("%s: Primary key expression is <%s> key length %d\n",
		t->name, g->pk_columns, g->pk_column_len);

    return g->pk_column_count;
}

int get_server_nrows (db_thread *t, node *n)
{
/*
 *     $int      nrows;
 */
#line 907 "database.ec"
int nrows;
/*
 *     $char     sql_stmt[BUF_SIZE];
 */
#line 908 "database.ec"
  char sql_stmt[BUF_SIZE];
    global   *g;

    g = t->globals;

/*
 *     $SET PDQPRIORITY 100;
 */
#line 913 "database.ec"
{
#line 913 "database.ec"
static const char *sqlcmdtxt[] =
#line 913 "database.ec"
{
#line 913 "database.ec"
"SET PDQPRIORITY 100",
0
};
#line 913 "database.ec"
ifx_statement_t _SQ0;
#line 913 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 913 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 913 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 913 "database.ec"
}

    sprintf (sql_stmt, "SELECT count(*) FROM %s@%s:%s",
	     g->database, n->name, g->table);

/*
 *     $PREPARE get_sts_count_id FROM $sql_stmt;
 */
#line 918 "database.ec"
{
#line 918 "database.ec"
sqli_prep(ESQLINTVERSION, _Cn8, sql_stmt,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 918 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 918 "database.ec"
}

/*
 *     $EXECUTE get_sts_count_id INTO $nrows;
 */
#line 920 "database.ec"
{
#line 920 "database.ec"
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
sqli_mt_obind(102, (char *) &nrows, sizeof(nrows), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn8, 769), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, _SD0, (char *)0, (struct value *)0, 0);
#line 920 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 920 "database.ec"
}

/*
 *     $FREE get_sts_count_id;
 */
#line 922 "database.ec"
{
#line 922 "database.ec"
  sqli_mt_free(_Cn8);
#line 922 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 922 "database.ec"
}

/*
 *     $SET PDQPRIORITY 0;
 */
#line 924 "database.ec"
{
#line 924 "database.ec"
static const char *sqlcmdtxt[] =
#line 924 "database.ec"
{
#line 924 "database.ec"
"SET PDQPRIORITY 0",
0
};
#line 924 "database.ec"
ifx_statement_t _SQ0;
#line 924 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 924 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 924 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 924 "database.ec"
}

    if (g->verbose)
	printf ("%s: There are %d known TimeSeries on server %s\n",
		t->name, nrows, n->name);

    return nrows;
}

int get_total_nrows (db_thread *t)
{
/*
 *     $int      nrows;
 */
#line 935 "database.ec"
int nrows;
/*
 *     $varchar *tabname;
 */
#line 936 "database.ec"
  char *tabname;
/*
 *     $char     sql_stmt[BUF_SIZE];
 */
#line 937 "database.ec"
  char sql_stmt[BUF_SIZE];
    char      grid_expr[128+16] = "";
    global   *g;
    int       total_nrows;

    g = t->globals;

    /* If running in clustered mode we should already have the total from
       when the node information was gathered */
    if (g->grid && g->total_nrows > 0)
	return g->total_nrows;

    /* Otherwise use a grid query to check all servers */
/*
 *     $SET PDQPRIORITY 100;
 */
#line 950 "database.ec"
{
#line 950 "database.ec"
static const char *sqlcmdtxt[] =
#line 950 "database.ec"
{
#line 950 "database.ec"
"SET PDQPRIORITY 100",
0
};
#line 950 "database.ec"
ifx_statement_t _SQ0;
#line 950 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 950 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 950 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 950 "database.ec"
}

    if (g->meta_table)
	tabname = g->meta_table;
    else
	tabname = g->table;

    if (g->grid)
	sprintf (grid_expr, " GRID ALL '%s'", g->grid);

    sprintf (sql_stmt, "SELECT count(*) FROM %s%s", tabname, grid_expr);

/*
 *     $PREPARE get_ts_count_id FROM $sql_stmt;
 */
#line 962 "database.ec"
{
#line 962 "database.ec"
sqli_prep(ESQLINTVERSION, _Cn9, sql_stmt,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 962 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 962 "database.ec"
}

/*
 *     $DECLARE get_ts_count CURSOR FOR get_ts_count_id;
 */
#line 964 "database.ec"
{
#line 964 "database.ec"
sqli_curs_decl_dynm(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn10, 512), _Cn10, sqli_curs_locate(ESQLINTVERSION, _Cn9, 513), 0, 0);
#line 964 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 964 "database.ec"
}

/*
 *     $OPEN get_ts_count;
 */
#line 966 "database.ec"
{
#line 966 "database.ec"
sqli_curs_open(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn10, 768), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0, 0);
#line 966 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 966 "database.ec"
}

    total_nrows = 0;
    while (sqlca.sqlcode == 0)
    {
/*
 * 	$FETCH get_ts_count INTO $nrows;
 */
#line 971 "database.ec"
{
#line 971 "database.ec"
ifx_sqlda_t *_SD0;
static _FetchSpec _FS1 = { 0, 1, 0 };
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
sqli_mt_obind(102, (char *) &nrows, sizeof(nrows), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_curs_fetch(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn10, 768), (ifx_sqlda_t *)0, _SD0, (char *)0, &_FS1);
#line 971 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 971 "database.ec"
}

	if (sqlca.sqlcode == 0)
	{
	    total_nrows += nrows;
	}
    }
/*
 *     $CLOSE get_ts_count;
 */
#line 978 "database.ec"
{
#line 978 "database.ec"
sqli_curs_close(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn10, 768));
#line 978 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 978 "database.ec"
}
/*
 *     $FREE get_ts_count;
 */
#line 979 "database.ec"
{
#line 979 "database.ec"
  sqli_mt_free(_Cn10);
#line 979 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 979 "database.ec"
}
/*
 *     $FREE get_ts_count_id;
 */
#line 980 "database.ec"
{
#line 980 "database.ec"
  sqli_mt_free(_Cn9);
#line 980 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 980 "database.ec"
}

/*
 *     $SET PDQPRIORITY 0;
 */
#line 982 "database.ec"
{
#line 982 "database.ec"
static const char *sqlcmdtxt[] =
#line 982 "database.ec"
{
#line 982 "database.ec"
"SET PDQPRIORITY 0",
0
};
#line 982 "database.ec"
ifx_statement_t _SQ0;
#line 982 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 982 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 982 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 982 "database.ec"
}

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
/*
 *     $varchar  cont_name[128+1];
 */
#line 1072 "database.ec"
  char cont_name[129];
/*
 *     $char     sql_stmt[BUF_SIZE];
 */
#line 1073 "database.ec"
  char sql_stmt[BUF_SIZE];
/*
 *     $long     instanceid = 0;
 */
#line 1074 "database.ec"
long instanceid = 0;
/*
 *     $int      hertz = 0;
 */
#line 1075 "database.ec"
int hertz = 0;

    n = &(g->server_list[t->connection->current_server]);

    match = key_match_expr (t, g->pk_column_count, g->pk_columns, key);

    sprintf (sql_stmt,
	     "SELECT TRIM(GetContainerName(%s)), GetHertz(%s) FROM %s WHERE %s",
	     g->column, g->column, g->table, match);

    free (match);

    if (g->verbose > 1)
	printf ("%s: EXEC-SQL %s;\n", t->name, sql_stmt);

/*
 *     $PREPARE get_1ts_info_id FROM $sql_stmt;
 */
#line 1090 "database.ec"
{
#line 1090 "database.ec"
sqli_prep(ESQLINTVERSION, _Cn11, sql_stmt,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 1090 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1090 "database.ec"
}

/*
 *     $DECLARE get_1ts_info CURSOR FOR get_1ts_info_id;
 */
#line 1092 "database.ec"
{
#line 1092 "database.ec"
sqli_curs_decl_dynm(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn12, 512), _Cn12, sqli_curs_locate(ESQLINTVERSION, _Cn11, 513), 0, 0);
#line 1092 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1092 "database.ec"
}

/*
 *     $OPEN get_1ts_info;
 */
#line 1094 "database.ec"
{
#line 1094 "database.ec"
sqli_curs_open(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn12, 768), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0, 0);
#line 1094 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1094 "database.ec"
}

    count = 0;
    while (sqlca.sqlcode == 0)
    {
/*
 * 	$FETCH get_1ts_info INTO $cont_name, $hertz;
 */
#line 1099 "database.ec"
{
#line 1099 "database.ec"
ifx_sqlda_t *_SD0;
static _FetchSpec _FS1 = { 0, 1, 0 };
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,2);
_dummy_SD0 (_SD0);
sqli_mt_obind(114, cont_name, 129, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &hertz, sizeof(hertz), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_curs_fetch(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn12, 768), (ifx_sqlda_t *)0, _SD0, (char *)0, &_FS1);
#line 1099 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1099 "database.ec"
}
	
	if (sqlca.sqlcode == 0)
	{
	    count++;
	    d = store_ts_data (t, instanceid, n, cont_name, (char *)key, 0, hertz, d);
	}
    }	
/*
 *     $CLOSE get_1ts_info;
 */
#line 1107 "database.ec"
{
#line 1107 "database.ec"
sqli_curs_close(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn12, 768));
#line 1107 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1107 "database.ec"
}

/*
 *     $FREE get_1ts_info;
 */
#line 1109 "database.ec"
{
#line 1109 "database.ec"
  sqli_mt_free(_Cn12);
#line 1109 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1109 "database.ec"
}
/*
 *     $FREE get_1ts_info_id;
 */
#line 1110 "database.ec"
{
#line 1110 "database.ec"
  sqli_mt_free(_Cn11);
#line 1110 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1110 "database.ec"
}

    if (count > 1 && g->verbose)
	printf ("%s: Unexpected - got %d rows from get_1ts_info\n",
		t->name, count);

    return d;
}

ts_data *insert_new_ts (db_thread *t, unsigned char *key, char *ts_spec, ts_data *d)
{
    global    *g = t->globals;
/*
 *     $char      sql_stmt[BUF_SIZE];
 */
#line 1122 "database.ec"
  char sql_stmt[BUF_SIZE];
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

/*
 *     $whenever error continue;
 */
#line 1153 "database.ec"

/*
 *     $execute immediate $sql_stmt;
 */
#line 1155 "database.ec"
{
#line 1155 "database.ec"
sqli_exec_immed(sql_stmt);
#line 1155 "database.ec"
}

    if (sqlca.sqlcode != 0 && g->verbose)
	printf ("ignore error %d from INSERT\n", sqlca.sqlcode);

/*
 *     $whenever error call SQL_Check;
 */
#line 1160 "database.ec"

    return load_key_info (t, key, d);
}

int get_keys (db_thread *t)
{
/*
 *     $varchar  all_cols[(390*4)+16];
 */
#line 1167 "database.ec"
  char all_cols[1576];
/*
 *     $varchar  cont_name[128+1];
 */
#line 1168 "database.ec"
  char cont_name[129];
/*
 *     $char     sql_stmt[BUF_SIZE];
 */
#line 1169 "database.ec"
  char sql_stmt[BUF_SIZE];
/*
 *     $long     instanceid;
 */
#line 1170 "database.ec"
long instanceid;
/*
 *     $int      hertz, nodeid;
 */
#line 1171 "database.ec"
int hertz, nodeid;

    node     *n;
    global   *g;

/*
 *     $SET PDQPRIORITY 100;
 */
#line 1176 "database.ec"
{
#line 1176 "database.ec"
static const char *sqlcmdtxt[] =
#line 1176 "database.ec"
{
#line 1176 "database.ec"
"SET PDQPRIORITY 100",
0
};
#line 1176 "database.ec"
ifx_statement_t _SQ0;
#line 1176 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 1176 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 1176 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1176 "database.ec"
}

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
	
/*
 * 	    $EXECUTE IMMEDIATE $sql_stmt;
 */
#line 1201 "database.ec"
{
#line 1201 "database.ec"
sqli_exec_immed(sql_stmt);
#line 1201 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1201 "database.ec"
}
	
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

/*
 *     $PREPARE get_ts_info_id FROM $sql_stmt;
 */
#line 1232 "database.ec"
{
#line 1232 "database.ec"
sqli_prep(ESQLINTVERSION, _Cn13, sql_stmt,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 1232 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1232 "database.ec"
}

/*
 *     $DECLARE get_ts_info CURSOR FOR get_ts_info_id;
 */
#line 1234 "database.ec"
{
#line 1234 "database.ec"
sqli_curs_decl_dynm(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn14, 512), _Cn14, sqli_curs_locate(ESQLINTVERSION, _Cn13, 513), 0, 0);
#line 1234 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1234 "database.ec"
}

/*
 *     $OPEN get_ts_info;
 */
#line 1236 "database.ec"
{
#line 1236 "database.ec"
sqli_curs_open(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn14, 768), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0, 0);
#line 1236 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1236 "database.ec"
}

    while (sqlca.sqlcode == 0)
    {
/*
 * 	$FETCH get_ts_info
 * 	    INTO $all_cols, $instanceid, $nodeid, $cont_name, $hertz;
 */
#line 1240 "database.ec"
{
#line 1241 "database.ec"
ifx_sqlda_t *_SD0;
static _FetchSpec _FS1 = { 0, 1, 0 };
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,5);
_dummy_SD0 (_SD0);
sqli_mt_obind(114, all_cols, 1576, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(103, (char *) &instanceid, sizeof(instanceid), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &nodeid, sizeof(nodeid), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(114, cont_name, 129, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &hertz, sizeof(hertz), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_curs_fetch(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn14, 768), (ifx_sqlda_t *)0, _SD0, (char *)0, &_FS1);
#line 1241 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1241 "database.ec"
}
	
	if (sqlca.sqlcode == 0)
	{
	    store_ts_data (t, instanceid, n, cont_name, all_cols, nodeid, hertz, NULL);
	}
	
    }	
/*
 *     $CLOSE get_ts_info;
 */
#line 1249 "database.ec"
{
#line 1249 "database.ec"
sqli_curs_close(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn14, 768));
#line 1249 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1249 "database.ec"
}

/*
 *     $FREE get_ts_info;
 */
#line 1251 "database.ec"
{
#line 1251 "database.ec"
  sqli_mt_free(_Cn14);
#line 1251 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1251 "database.ec"
}
/*
 *     $FREE get_tsinfo_id;
 */
#line 1252 "database.ec"
{
#line 1252 "database.ec"
  sqli_mt_free(_Cn15);
#line 1252 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1252 "database.ec"
}

/*
 *     $SET PDQPRIORITY 0;
 */
#line 1254 "database.ec"
{
#line 1254 "database.ec"
static const char *sqlcmdtxt[] =
#line 1254 "database.ec"
{
#line 1254 "database.ec"
"SET PDQPRIORITY 0",
0
};
#line 1254 "database.ec"
ifx_statement_t _SQ0;
#line 1254 "database.ec"
byfill((char *) &_SQ0, sizeof(_SQ0), 0);
#line 1254 "database.ec"
sqli_stmt(ESQLINTVERSION, &_SQ0, sqlcmdtxt, 0, (ifx_sqlvar_t *) 0, (struct value *)0, (ifx_literal_t *)0, (ifx_namelist_t *)0, (ifx_cursor_t *)0, -1, 0, 0);
#line 1254 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1254 "database.ec"
}

    if (g->verbose)
	printf ("%s: Retrieved %d of %d active TimeSeries from node %s, %d have moved\n",
		t->name, n->next_ts_data, n->nrows, n->name, n->displaced);

    return n->next_ts_data;
}

int db_flush_data (db_thread *t)
{
/*
 *     $int      rval = 0, flag, i, txsize;
 */
#line 1265 "database.ec"
int rval = 0, flag, i, txsize;
/*
 *     $varchar *s_handle;
 */
#line 1266 "database.ec"
  char *s_handle;
/*
 *     $db_prep *p = t->connection->prepared;
 */
#line 1267 "database.ec"
db_prep *p = t->connection->prepared;

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

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 1284 "database.ec"

    if (g->lock_containers || p->commit_data_id == NULL)
    {
/*
 * 	$BEGIN;
 */
#line 1288 "database.ec"
{
#line 1288 "database.ec"
sqli_trans_begin2((mint)1);
#line 1288 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1288 "database.ec"
}

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

/*
 * 	$EXECUTE $p->flush_data_id INTO $rval USING $s_handle, $flag;
 */
#line 1304 "database.ec"
{
#line 1304 "database.ec"
ifx_sqlda_t *_SD0;
ifx_sqlda_t *_SD1;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,2);
_dummy_SD0 (_SD0);
_SD1 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
sqli_mt_ibind(114, s_handle, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(102, (char *) &flag, sizeof(flag), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &rval, sizeof(rval), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->flush_data_id, 257), _SD0, (char *)0, (struct value *)0, _SD1, (char *)0, (struct value *)0, 0);
#line 1304 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1304 "database.ec"
}

/*
 * 	$COMMIT;
 */
#line 1306 "database.ec"
{
#line 1306 "database.ec"
sqli_trans_commit();
#line 1306 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1306 "database.ec"
}

	/* This is not set by the info function as the commit occurs
	   outside the scope of the flush. */
	t->commits++;
    }
    else
    {
/*
 * 	$short txsizeind = 0;
 */
#line 1314 "database.ec"
short txsizeind = 0;
	
	if ((txsize = g->txsize) == -1)
	    txsizeind = -1;

/*
 * 	$EXECUTE $p->commit_data_id INTO $rval
 * 	    USING $s_handle, $flag, $txsize:txsizeind;
 */
#line 1319 "database.ec"
{
#line 1320 "database.ec"
ifx_sqlda_t *_SD0;
ifx_sqlda_t *_SD1;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,3);
_dummy_SD0 (_SD0);
_SD1 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
sqli_mt_ibind(114, s_handle, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(102, (char *) &flag, sizeof(flag), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(102, (char *) &txsize, sizeof(txsize), 101, (char *) &txsizeind, sizeof(txsizeind), 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &rval, sizeof(rval), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->commit_data_id, 257), _SD0, (char *)0, (struct value *)0, _SD1, (char *)0, (struct value *)0, 0);
#line 1320 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1320 "database.ec"
}
    }

    if (g->collect_info)
    {
/*
 * 	$int  containers, elements, duplicates, instance_ids,
 * 	    commits, rollbacks, exceptions, errors;
 */
#line 1325 "database.ec"
int containers, elements, duplicates, instance_ids, commits, rollbacks, exceptions, errors;

/*
 * 	$EXECUTE $p->info_id
 * 	    INTO $containers, $elements, $duplicates, $instance_ids,
 * 	    $commits, $rollbacks, $exceptions, $errors USING $s_handle;
 */
#line 1328 "database.ec"
{
#line 1330 "database.ec"
ifx_sqlda_t *_SD0;
ifx_sqlda_t *_SD1;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
_SD1 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,8);
sqli_mt_ibind(114, s_handle, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &containers, sizeof(containers), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &elements, sizeof(elements), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &duplicates, sizeof(duplicates), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &instance_ids, sizeof(instance_ids), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &commits, sizeof(commits), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &rollbacks, sizeof(rollbacks), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &exceptions, sizeof(exceptions), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &errors, sizeof(errors), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->info_id, 257), _SD0, (char *)0, (struct value *)0, _SD1, (char *)0, (struct value *)0, 0);
#line 1330 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1330 "database.ec"
}
	
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
/*
 *     $int      response;
 */
#line 1352 "database.ec"
int response;
/*
 *     $varchar *s_handle = t->globals->server_list[t->connection->current_server].handle;
 */
#line 1353 "database.ec"
  char *s_handle = t->globals->server_list[t->connection->current_server].handle;
/*
 *     $db_prep *p = t->connection->prepared;
 */
#line 1354 "database.ec"
db_prep *p = t->connection->prepared;

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 1356 "database.ec"
	
/*
 *     $EXECUTE $p->release_id INTO $response USING $s_handle;
 */
#line 1358 "database.ec"
{
#line 1358 "database.ec"
ifx_sqlda_t *_SD0;
ifx_sqlda_t *_SD1;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
_SD1 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,1);
sqli_mt_ibind(114, s_handle, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(102, (char *) &response, sizeof(response), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->release_id, 257), _SD0, (char *)0, (struct value *)0, _SD1, (char *)0, (struct value *)0, 0);
#line 1358 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1358 "database.ec"
}

    return response;
}

int tsl_shutdown (db_thread *t)
{
/*
 *     $int      response;
 */
#line 1365 "database.ec"
int response;
/*
 *     $varchar *s_handle = t->globals->server_list[t->connection->current_server].handle;
 */
#line 1366 "database.ec"
  char *s_handle = t->globals->server_list[t->connection->current_server].handle;
/*
 *     $db_prep *p = t->connection->prepared;
 */
#line 1367 "database.ec"
db_prep *p = t->connection->prepared;

    response = tsl_release (t);

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 1371 "database.ec"
	
/*
 *     $EXECUTE $p->shutdown_id USING $s_handle;
 */
#line 1373 "database.ec"
{
#line 1373 "database.ec"
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,1);
_dummy_SD0 (_SD0);
sqli_mt_ibind(114, s_handle, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->shutdown_id, 257), _SD0, (char *)0, (struct value *)0, (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0);
#line 1373 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1373 "database.ec"
}

    return response;
}

void free_stmt_id (char **id)
{
/*
 *     $varchar *stmt_id = *id;
 */
#line 1380 "database.ec"
  char *stmt_id = *id;

/*
 *     $FREE $stmt_id;
 */
#line 1382 "database.ec"
{
#line 1382 "database.ec"
  sqli_mt_free(stmt_id);
#line 1382 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1382 "database.ec"
}
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
/*
 *     $db_prep  *p = c->prepared;
 */
#line 1438 "database.ec"
db_prep *p = c->prepared;

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 1440 "database.ec"

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
	
/*
 *     $DISCONNECT $p->connection_id;
 */
#line 1475 "database.ec"
{
#line 1475 "database.ec"
sqli_connect_close(0, p->connection_id, 0, 0);
#line 1475 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1475 "database.ec"
}

    c->connected = 0;

    return 0;
}

int db_insert_storage (db_thread *t,
/*
 * 		       EXEC SQL BEGIN DECLARE SECTION;
 */
#line 1483 "database.ec"
#line 1484 "database.ec"
  char *pk,
  char *tstamp,
/*
 * 		       EXEC SQL END DECLARE SECTION;
 */
#line 1486 "database.ec"

		       char *values)
{

/*
 *     $int      val = strtol (values, NULL, 0);
 */
#line 1490 "database.ec"
int val = strtol (values, NULL, 0);
/*
 *     $db_prep *p = t->connection->prepared;
 */
#line 1491 "database.ec"
db_prep *p = t->connection->prepared;

    if (t->globals->verbose > 0)
	printf ("%s; Inserting (%s, %s, %d)\n", t->name, pk, tstamp, val);
		
/*
 *     $EXECUTE $p->storage_id USING $pk, $tstamp, $val;
 */
#line 1496 "database.ec"
{
#line 1496 "database.ec"
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_isqlda(ESQLINTVERSION,3);
_dummy_SD0 (_SD0);
sqli_mt_ibind(114, pk, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(114, tstamp, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_ibind(102, (char *) &val, sizeof(val), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->storage_id, 257), _SD0, (char *)0, (struct value *)0, (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0);
#line 1496 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1496 "database.ec"
}

    return sqlca.sqlcode;
}

long db_next_device (
    db_thread *t,
/*
 *     EXEC SQL BEGIN DECLARE SECTION;
 */
#line 1503 "database.ec"
#line 1504 "database.ec"
  char *container,
short hertz
/*
 *     EXEC SQL END DECLARE SECTION;
 */
#line 1506 "database.ec"

    )
{
/*
 *     $long      s_devid;
 */
#line 1509 "database.ec"
long s_devid;
/*
 *     $db_prep  *p = t->connection->prepared;
 */
#line 1510 "database.ec"
db_prep *p = t->connection->prepared;

    if (t->globals->verbose > 0)
	printf ("%s; Getting next_device()\n", t->name);
		
/*
 *     $EXECUTE $p->next_device_id INTO $s_devid, $container, $hertz;
 */
#line 1515 "database.ec"
{
#line 1515 "database.ec"
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,3);
_dummy_SD0 (_SD0);
sqli_mt_obind(103, (char *) &s_devid, sizeof(s_devid), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(114, container, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(101, (char *) &hertz, sizeof(hertz), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, p->next_device_id, 257), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, _SD0, (char *)0, (struct value *)0, 0);
#line 1515 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1515 "database.ec"
}

    return s_devid;
}

long db_next_displaced (db_thread *t, char *key)
{
/*
 *     $long      s_devid;
 */
#line 1522 "database.ec"
long s_devid;
/*
 *     $varchar   stmt[BUF_SIZE];
 */
#line 1523 "database.ec"
  char stmt[BUF_SIZE];
/*
 *     $varchar  *s_key = key;
 */
#line 1524 "database.ec"
  char *s_key = key;
    global    *g;

    g = t->globals;

    if (g->verbose > 0)
	printf ("%s; Getting next_device()\n", t->name);
		
    sprintf (stmt, "SELECT FIRST 1 deviceid, devicename FROM %s WHERE %s != %d",
	     g->meta_table, g->meta_nodeid, g->server_list[t->connection->current_server].id);

    if (g->verbose > 0)
	printf ("%s: EXEC-SQL %s\n", t->name, stmt);

/*
 *     $PREPARE stmt_id FROM $stmt;
 */
#line 1538 "database.ec"
{
#line 1538 "database.ec"
sqli_prep(ESQLINTVERSION, _Cn16, stmt,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0 ); 
#line 1538 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1538 "database.ec"
}

/*
 *     $EXECUTE stmt_id INTO $s_devid, $s_key;
 */
#line 1540 "database.ec"
{
#line 1540 "database.ec"
ifx_sqlda_t *_SD0;
_SD0 = (ifx_sqlda_t *) sqli_mt_alloc_osqlda(ESQLINTVERSION,2);
_dummy_SD0 (_SD0);
sqli_mt_obind(103, (char *) &s_devid, sizeof(s_devid), 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_mt_obind(114, s_key, 0, 0, 0, 0, 0, (char *)0, 0, (char *)0, 0, (char *)0, 0, 0);
sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn16, 769), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, _SD0, (char *)0, (struct value *)0, 0);
#line 1540 "database.ec"
{ if (SQLCODE < 0) SQL_Check(); }
#line 1540 "database.ec"
}

    if (g->verbose > 0)
	printf ("%s: Next device to be moved %ld\n", t->name, s_devid);

    return s_devid;
}

int db_move_ts (db_thread *t, long old_id, long new_id, int server)
{
/*
 *     $char     s_sql[BUF_SIZE];
 */
#line 1550 "database.ec"
  char s_sql[BUF_SIZE];

    global     *g;
    node       *n;

    g = t->globals;
    n = &(g->server_list[server]);

    sprintf (s_sql, "EXECUTE FUNCTION transfer_ts (%ld, '%s', %ld)",
	     new_id, n->name, old_id);

    if (g->verbose > 0)
	printf ("%s: EXEC-SQL %s;\n", t->name, s_sql);

/*
 *     $WHENEVER ERROR CALL SQL_Check;
 */
#line 1564 "database.ec"

    return 0;
}

#line 1567 "database.ec"
