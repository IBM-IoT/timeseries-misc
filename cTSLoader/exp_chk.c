#include <string.h>
#include <sqlhdr.h>
#include <sqliapi.h>
#line 1 "exp_chk.ec"
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
 *  Title            : exp_chk.ec
 *  Description      : ESQL/C error handling
 *
 ****************************************************************************
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define LOG 	printf

/* 
 * EXEC SQL define SUCCESS 0;
 */
#line 24 "exp_chk.ec"

/* 
 * EXEC SQL define WARNING 1;
 */
#line 25 "exp_chk.ec"

/* 
 * EXEC SQL define NODATA 100;
 */
#line 26 "exp_chk.ec"

/* 
 * EXEC SQL define RTERROR -1;
 */
#line 27 "exp_chk.ec"


/*
 * The sqlstate_err() function checks the SQLSTATE status variable to see
 * if an error or warning has occurred following an SQL statement.
 */
int sqlstate_err()
{
    int err_code = -1;

    if(SQLSTATE[0] == '0') /* trap '00', '01', '02' */
        {
        switch(SQLSTATE[1])
            {
            case '0': /* success - return 0 */
                err_code = 0;
                break;
            case '1': /* warning - return 1 */
                err_code = 1;
                break;
            case '2': /* end of data - return 100 */
                err_code = 100;
                break;
            default: /* error - return SQLCODE */
                break;
            }
        }
    return(err_code);
}


/*
 * The disp_sqlstate_err() function executes the GET DIAGNOSTICS
 * statement and prints the detail for each exception that is returned.
 */
void disp_sqlstate_err()
{
int j;

/*
 * EXEC SQL BEGIN DECLARE SECTION;
 */
#line 66 "exp_chk.ec"
#line 67 "exp_chk.ec"
int exception_count;
  char overflow[2];
  char class_id[255];
  char subclass_id[255];
  char message[255];
int messlen;
  char sqlstate_code[6];
int i;
/*
 * EXEC SQL END DECLARE SECTION;
 */
#line 75 "exp_chk.ec"


    LOG("---------------------------------");
    LOG("-------------------------\n");
    LOG("SQLSTATE: %s\n",SQLSTATE);
    LOG("SQLCODE: %d\n", SQLCODE);
    LOG("\n");

/*
 *     EXEC SQL get diagnostics :exception_count = NUMBER,
 *         :overflow = MORE;
 */
#line 83 "exp_chk.ec"
  {
#line 84 "exp_chk.ec"
  ifx_hostvar_t _SQhtab[3];
  byfill((char *) &_SQhtab, (sizeof(ifx_hostvar_t) * 3), 0);
  sqli_mt_hostbind(ESQLINTVERSION, &_SQhtab[0], 1, 102, 0, sizeof(exception_count), 0);
  _SQhtab[0].hostaddr = (char *)&exception_count;
  sqli_mt_hostbind(ESQLINTVERSION, &_SQhtab[1], 2, 100, 0, 2, 0);
  _SQhtab[1].hostaddr = (overflow);
#line 84 "exp_chk.ec"
  sqli_diag_get(ESQLINTVERSION, _SQhtab, -1);
#line 84 "exp_chk.ec"
  }
    LOG("EXCEPTIONS:  Number=%d\t", exception_count);
    LOG("More? %s\n", overflow);
    for (i = 1; i <= exception_count; i++)
        {
/*
 *         EXEC SQL get diagnostics  exception :i
 *             :sqlstate_code = RETURNED_SQLSTATE,
 *             :class_id = CLASS_ORIGIN, :subclass_id = SUBCLASS_ORIGIN,
 *             :message = MESSAGE_TEXT, :messlen = MESSAGE_LENGTH;
 */
#line 89 "exp_chk.ec"
  {
#line 92 "exp_chk.ec"
  ifx_hostvar_t _SQhtab[6];
  byfill((char *) &_SQhtab, (sizeof(ifx_hostvar_t) * 6), 0);
  sqli_mt_hostbind(ESQLINTVERSION, &_SQhtab[0], 3, 100, 0, 6, 0);
  _SQhtab[0].hostaddr = (sqlstate_code);
  sqli_mt_hostbind(ESQLINTVERSION, &_SQhtab[1], 4, 100, 0, 255, 0);
  _SQhtab[1].hostaddr = (class_id);
  sqli_mt_hostbind(ESQLINTVERSION, &_SQhtab[2], 5, 100, 0, 255, 0);
  _SQhtab[2].hostaddr = (subclass_id);
  sqli_mt_hostbind(ESQLINTVERSION, &_SQhtab[3], 6, 100, 0, 255, 0);
  _SQhtab[3].hostaddr = (message);
  sqli_mt_hostbind(ESQLINTVERSION, &_SQhtab[4], 7, 102, 0, sizeof(messlen), 0);
  _SQhtab[4].hostaddr = (char *)&messlen;
#line 92 "exp_chk.ec"
  sqli_diag_get(ESQLINTVERSION, _SQhtab, i);
#line 92 "exp_chk.ec"
  }
        LOG("- - - - - - - - - - - - - - - - - - - -\n");
        LOG("EXCEPTION %d: SQLSTATE=%s\n", i,
            sqlstate_code);
        message[messlen-1] = '\0';
        LOG("MESSAGE TEXT: %s\n", message);

        j = byleng(class_id, stleng(class_id));
        class_id[j] = '\0';
        LOG("CLASS ORIGIN: %s\n",class_id);

        j = byleng(subclass_id, stleng(subclass_id));
        subclass_id[j] = '\0';
        LOG("SUBCLASS ORIGIN: %s\n",subclass_id);
        }

    LOG("---------------------------------");
    LOG("-------------------------\n");
}

char *t_name(int *id);

void disp_error(stmt)
char *stmt;
{
    int     tid;
    char   *tname = t_name (&tid);
    LOG ("\nThread[%d]: %s\n", tid, tname);
    LOG("\n********Error encountered in %s********\n",
        stmt);
    disp_sqlstate_err();
}

void disp_warning(stmt)
char *stmt;
{
    LOG("\n********Warning encountered in %s********\n",
        stmt);
    disp_sqlstate_err();
}

void disp_exception(stmt, sqlerr_code, warn_flg)
char *stmt;
int sqlerr_code;
int warn_flg;
{
    switch (sqlerr_code)
        {
        case 0:
        case 100:
            break;
        case 1:
            if(warn_flg)
                disp_warning(stmt);
            break;
        case -1:
            disp_error(stmt);
            break;
        default:
            LOG("\n********INVALID EXCEPTION STATE for %s********\n",
                stmt);
            break;
        }
}

/*
 * The exp_chk() function calls sqlstate_err() to check the SQLSTATE
 * status variable to see if an error or warning has occurred following
 * an SQL statement. If either condition has occurred, exp_chk()
 * calls disp_sqlstate_err() to print the detailed error information.
 *
 * This function handles exceptions as follows:
 *   runtime errors - call exit(1)
 *   warnings - continue execution, returning "1"
 *   success - continue execution, returning "0"
 *   Not Found - continue execution, returning "100"
 */
long exp_chk(stmt, warn_flg)
char *stmt;
int warn_flg;
{
    int sqlerr_code = 0;

    sqlerr_code = sqlstate_err();
    disp_exception(stmt, sqlerr_code, warn_flg);

    if(sqlerr_code == -1)   
        {
        /* Exit the program after examining the error */
        LOG("********Program terminated*******\n\n");
        exit(1);
        }
    else                        /* Exception is "success", "Not Found",*/
        return(sqlerr_code);    /*  or "warning"                       */
}

/*
 * The exp_chk2() function calls sqlstate_err() to check the SQLSTATE
 * status variable to see if an error or warning has occurred following
 * an SQL statement. If either condition has occurred, exp_chk2()
 * calls disp_exception() to print the detailed error information.
 *
 * This function handles exceptions as follows:
 *   runtime errors - continue execution, returning SQLCODE (<0)
 *   warnings - continue execution, returning one (1)
 *   success - continue execution, returning zero (0)
 *   Not Found - continue execution, returning 100
 */
long exp_chk2(stmt, warn_flg)
char *stmt;
int warn_flg;
{
    int sqlerr_code = 0;
    long sqlcode;

    sqlcode = SQLCODE;  /* save SQLCODE in case of error */
    sqlerr_code = sqlstate_err();
    disp_exception(stmt, sqlerr_code, warn_flg);

    if(sqlerr_code == -1)  
        sqlerr_code = sqlcode;

    return(sqlerr_code);
}

/*
 * The whenexp_chk() function calls sqlstate_err() to check the SQLSTATE
 * status variable to see if an error or warning has occurred following
 * an SQL statement. If either condition has occurred, whenerr_chk()
 * calls disp_sqlstate_err() to print the detailed error information.
 *
 * This function is expected to be used with the WHENEVER SQLERROR
 * statement: it executes an exit(1) when it encounters a negative
 * error code.
*/
int whenexp_chk_line(int _line_no_)
{
    int sqlerr_code = 0;
    int disp = 0;
    int tid;
    char *tname = t_name (&tid);

    sqlerr_code = sqlstate_err();

    LOG ("\nThread[%d]: %s\n", tid, tname);

    if(sqlerr_code == 1)
        {
        disp = 1;
        LOG("\n******** Warning encountered at line %d ********\n",
            _line_no_);
        }
    else
        if(sqlerr_code == -1)
            {
            LOG("\n******** Error encountered at line %d ********\n",
                _line_no_);
            disp = 1;
            }
    if(disp)
        disp_sqlstate_err();

    if(sqlerr_code == -1)
        {
        /* Exit the program after examining the error */
        LOG("******** Program terminated *******\n\n");
        exit(1);
        }
    else
        {
	  if(sqlerr_code == 1)
            LOG("\n******** Program execution continues ********\n\n");
        return(sqlerr_code);
        }
    return 0;
}

void _dummy(){ return; }

void whenexp_chk ()
{
    (void) whenexp_chk_line (-1);
}

#line 274 "exp_chk.ec"
