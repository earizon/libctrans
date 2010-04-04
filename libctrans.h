/* LIBCTRANS - Library of useful routines for transaction oriended programming in C 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Created by Enrique Ariz'on Benito, 2010.  
 */

/** @file libctrans.h
 *  \mainpage Abstract
 *  libctrans is designed to promote transaction oriented programming into plain
 *  C. It means that code is organized into logical "tasks" with an start and an
 *  end. As in a database, if an error/exception/abnormal execution arises the
 *  transaction can be rolled back to an stable state. 
 *
 *  As a free gift libctrans allows plain C to use code constructions similar to that
 *  of try-catch error handling in Java and other languages with other "goodies" not
 *  availaible in such languages.
 *
 *  libctrans is not transparent to the user (C programmer). Code has to be
 *  changed to take advantage of it. Still its ussage is quite intuitive and
 *  the performance is quite similar to normal C without transaction support:
 *  Basically it means a Transaction pointer must be passed to each function
 *  and wrapper functions used instead of standard ones. For example
 *  try_malloc transform into ctrans_try_malloc. Nevertheless, normal C can 
 *  be mixed if used with care and is NOT discouraged. Basically it means
 *  that any C code inside a transaction that makes not use of ctrans frees
 *  any used resource before the transaction ends (either normally
 *  or through an excption).
 *  
 *  While using an extra pointer in each call to a new function slow down
 *  the code, it gets compensated by the fact that there will be no need to
 *  check for errors at the return of the called function.
 *
 *  Next is a draft diagram showing the difference. In both case function4, placed
 *  5 levels down the stack can return/raise an error/exception.
 *
 *  <pre> 
 *  Normal C                          |       C using Transactions
 *                                    |                                  
 *  function mainloop                 |       function mainloop          
 *    invoque function1               |         STARTTRANSACTION         
 *    check return value for errors   |         invoque function1        
 *                                    |         ENDTRANSACTION           
 *                                    |                                  
 *  function1                         |       function1                  
 *    invoque function2               |         invoque function2        
 *    check return value for errors   |                                  
 *                                    |                                  
 *  function2                         |       function2                  
 *    invoque function3               |         invoque function3        
 *    check return value for errors   |                                  
 *                                    |                                  
 *  function3                         |       function3                  
 *    invoque function4               |         invoque function4        
 *    check return value for errors   |                                  
 *                                    |                                  
 *  function4                         |       function4                  
 *    return "OK" or "KO"             |         raise exception?         
 *                                    |                                  
 *                                    |       function transaction_start 
 *                                    |       function transaction_stop  
 *                                    |       function exception_captured
 *  </pre>
 *
 *  It quite clear from the diagram above that Transactions promote code 
 * easier to read, since error conditions are separated from normal flow
 * of code. 
 *
 *  Notice also in case of error in function 4 "Normal C" must go back 
 * the stack path from function4 to the mainloop while "C using Transactions"
 * will directly jump from function4 to the function "exception_captured", 
 * and start a new mainloop iteration. So in some 
 * circumstances transaction aware code will be faster than standard C
 * even if code have to bear with an extra pointer. 
 *
 * <b>Even more, transaction aware code will waste fewer blocks in RAM and 
 * that means it will make a better use of the internal CPU cache. 
 * Better use of the internal CPU cache can potencially boosts performance</b>.
 *
 *  <h1>Stupdly simple tutorial</h1>
 *  A transaction start with a code similar to:
 *  <pre>
 *    static Transaction* ptrTrans;
 *    NEWTRANSACTION(ptrTrans, transaction_start, transaction_stop, exception_captured, "Transaction Name");
 *  </pre>
 *  function transaction_start is executed before anything else in the transaction lifetime.<br>
 *  function transaction_stop is executed when we programatically indicate the end of transaction using ENDTRANSACTION.<br>
 *  function exception_captured is executed when we programatically raise an exception.<br>
 *
 *  Each NEWTRANSACTION(ptrTrans) must be matched with its partner:
 *  <pre>
 *       ENDTRANSACTION(ptrTrans);
 *  </pre>
 *  Optionally a transaction can be aborted using one of:
 *  <pre>
 *   ctrans_raise_recipient_exception(ptrTrans, 2, "description_i18n", "detail_i18n", "(posible)solution_i18n");
 *  </pre>
 *  or 
 *  <pre> 
 *  ctrans_raise_sender_exception( ptrTrans, 2, "description_i18n", "detail_i18n") ;
 *  </pre>
 *  Production code (context aware) must use ctrans_raise_recipient_exception (who is in charge of the exception).
 *  Library code (context unaware) must  use ctrans_raise_sender_exception (who is in charge of the exception).
 *
 *  The second argument (type) is defined for recipient exceptions and must be one of:
 *  typedef enum { USER=1100000, ADMIN=1200000, IMPLEMENTATION=1300000 } recipEx_type;
 *
 *  The second argument is any integer. At this moment (April 2010) no standard sender exception are defined.<br>
 *  Any sensible integer value can be used. <b>Nevertheless use any custom integer in the range 2000000-3000000</b><br>.
 *  Maybe it can make sense in a future to have standard type for common scenarios. For example, something like:
 *  <pre>
 *  typedef enum { IOEXCEPTION=1<<1, TIMEOUT=1<<2, NO_RESOURCE_AVAILABLE=1<<3, ... } senderEx_type; <br>
 *  </pre>
 *  Where the final exception could be the masked sum of standard exception plus maybe a custom integer like:<br>
 *  <pre>
 *  IOEXCEPTION || NO_RESOURCE_AVAILABLE
 *  </pre>
 *  From the author point of view that will make exceptions more intuitive and useful to handle than 
 *  the typed exceptions of Java or completly non-typed exceptions in other languages. All this without dropping the
 *  speed of pure C.<br>
 *
 *  The code will continue executing in the line below ENDTRANSACTION once transaction_stop or exception_captured are finished.
 *  That can look anoying in theory but it's not in practice. Just take a look at examples test*.c. The squema next sows
 *  how code will flow with or without exceptions in function 4.
 *
 *  <pre> 
 *                C using Transactions
 *                                           
 *                function mainloop                                     
 *  +-----<-------  STARTTRANSACTION                                    
 *  |               invoque function1 <---------+                       
 *  |  +----------  ENDTRANSACTION    <------+  |                       
 *  |  |            (normally loop end)      |  |  <----+               
 *  |  |                                     |  |       |               
 *  |  |          .....                      |  |       |               
 *  |  |          function4                  |  |       |               
 *  |  |   +- YES<- raise exception?  ->NO --+  |       |               
 *  |  |   |                                    |       |               
 *  |  |   |                                    |       |               
 *  +-----------> function transaction_start ->-+       |               
 *     |   |                                            |               
 *     +--------> function transaction_stop   ----->----+               
 *         |                                            |               
 *         +----> function exception_captured ----------+               
 *  </pre>
 *
 *  The first difference to notice respect to normal C is that transaction_start is executed before invoquing function1.
 *  When code reachs function4 and no exception is raised, code will jump back the stack and enter ENDTRANSACTION that will
 *  execute transaction_stop. Finally the instruction pointer will be placed after ENDTRANSACTION. In an exception is
 *  raised code will jump to exception_captured and finally the code will be placed after ENDTRANSACTION. <br>
 *  (If you look at the implementation of STARTTRANSACTION and ENDTRASACTION you will see that the previous paragraph is 
 *  completly false, but the pretended behaviour is the one shown in the diagram).<br>
 *  Any resources associated to the transaction will be automatically freed when calling ENDTRANSACTION or raising a new
 *  exception. Anything else will NOT.
 *  
 *  <h1>Design patterns</h1>
 *  <ul>
 *  <li>
 *  STARTTRANSACTION and ENDTRANSACTION must be placed in the same visibility scope. In fact the macro STARTTRANSACTION
 *  makes a hidden jump to a tag defined in ENDTRANSACTION so extrange compiler errors would arise.
 *  </li>
 *  <li>
 *  Ussually a function transaction_end will be used and called both in transaction_stop/exception_captured. That function will
 *  play a similar role to the "finally" code in the try-catch-finally Java syntax. It defines common code for cases
 *  that do/do-not raise an exception.
 *  </li>
 *
 *  For the impacients, <a href="./files.html">test*.c</a> provide example code of its use in code.
 */

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <glib-2.0/glib.h>

#ifndef __CTRANSACTIONS__
    #define __CTRANSACTIONS__


/**
 * @brief It's used as a parent class for sender/recipient exceptions.<br>
 * When a new exception is thrown the current transaction aborts and
 * the function associated with the trans. is executed.
 *
 * An union is used to implement child classes.
 * At this moment (2010-03) only the type, and *i18n are used.
 * Other members must be ignored.
 */
typedef struct {  
    guint32                           type;
    guint32                           serial_number;
    guint32                           parent_serial_number;
    guint64                           ms_timestamp;
    GThread*                          gthread;
    char*                             description_i18n;
    char*                             detail_i18n;
} exception_base;

/**
 * @brief Non-context aware exception. Used for libraries. 
 * 
 * Libraries are not aware of the context they run in.
 * A library doesn't know why an string is null or empty,
 * neither who is in charge of solving the error. It will
 * limit to raise a new exception indicating a problem.
 * 
 * At this moment (2010-03) its utility is limited, and
 * probably using GError will be a faster alternative.
 * (Citation needed)
 */
typedef struct {  
    union {
        exception_base parent;
    }                                 parent;
} sender_exception;

/**
 * @brief Context aware exception. Used for production code. 
 * 
 * Production code is aware of the conext it's running in.
 * Applications will try to classify the exception an send it
 * to the appropiate recipient.
 * type must be of type recipEx_type (USER,ADMIN,IMPLEMENTATION).
 */
typedef struct {  
    union {
        exception_base parent;
    }                                 parent;
    char* solution_i18n;
} recipient_exception;













#define TRANS_START        0
#define TRANS_STOP         1
#define EXCEPTION_CAPTURED 2




/**
 * @brief Represents a running transaction
 *
 *
 */
struct _Transaction {
    uint     id;
    guint32* stack_ptr2Top;
    guint32* stack_backup;
    guint    stack_size;
    jmp_buf  transStart;
    gchar*   sDebug; // free use for debugging purposes
    GList*   child_transactions ;
    GList*   allocated_memory   ;
    // Other possible transaction resources:
 // GList*   allocated_sockets  ;  ?  // TODO(1)
 // GList*   allocated_threads  ;  ?  // TODO(1)
 // GList*   allocated_timers   ;  ?  // TODO(1)
 // GList*   allocated_listeners;  ?  // TODO(1)
    exception_base* reisedException;
};

typedef struct _Transaction Transaction;

gint       ctrans_new_transaction (guint32* stack_ptr2Top, Transaction** ppTrans, 
               Transaction* pParentTrans, gchar* sDebug) ;
void       ctrans_finish_transaction(Transaction* pTrans) ;
gpointer   ctrans_try_malloc (Transaction* trans, gsize n_bytes, gboolean bRaiseException) ;
// TODO(1) ctrans_free_resources is probably private to transactions.c
void       ctrans_free_resources (Transaction* trans) ;

/** @brief Raises/throws a new sender exception.
 *
 * Sender exceptions will be used in libraries. Libraries
 * are not context-aware so they will limit to raise a new exception
 * with as many info as possible.
 * At the time of writing (2010-03) its utility is limited.
 * It's implemented just for simetry with the recipient_exceptions.
 * GError (http://library.gnome.org/devel/glib/2.23/glib-Error-Reporting.html)
 * could be enough for most purposes.
 * \param pTrans is the pointer to the active transaction.
 * \param type   provides simple and limited support to type the exception.
 * \param description_i18n human readable text 
 * \param detail_i18n  human readable text 
 */
void ctrans_raise_sender_exception(Transaction* pTrans,
    guint32 type, char* description_i18n, char* detail_i18n) ;

typedef enum { USER=1100000, ADMIN=1200000, IMPLEMENTATION=1300000 } recipEx_type;
/**
 * @brief Raises/throws a new recipient exception.
 *
 * Recipient exceptions will be used in production code. Such code
 * is context-aware so it will send the exception to a final target
 * (user, administrator). The implementation type is also available
 * but it actually its target will be an external bug tracking system.
 * 
 * Ussually a main loop will catch the recipient exception and act
 * sensible (sending a polity warning to the user or an SMS to the 
 * server administrator).
 *
 * \param pTrans is the pointer to the active transaction.
 * \param type   provides simple and limited support to type the exception.
 * \param description_i18n human readable text 
 * \param detail_i18n  human readable text 
 * \param solution_i18n  human readable text. The solution must really
 *     be a real solution or at least a set of hints or guides to 
 *     help solve the problem.
 *     That will make software maintenance much easier for the IT 
 *     department.
 */
void ctrans_raise_recipient_exception(Transaction* pTrans,
    recipEx_type type, char* description_i18n, char* detail_i18n, char* solution_i18n) ;

/*
 * TODO(0): Add a function FUN_END_COMMON (common to fun_stop and fun_exc)
 */

#define NEWTRANSACTION(POINTERTRANS, FUN_START, FUN_STOP, FUN_EXC, SDEBUG) \
    void (*exceptionHandle)  (Transaction*); \
    void (*transStartHandle) (Transaction*); \
    void (*transStopHandle)  (Transaction*); \
    guint32 stackTop; \
    exceptionHandle  = FUN_EXC; \
    transStartHandle = FUN_START; \
    transStopHandle  = FUN_STOP; \
    int tranState = ctrans_new_transaction(&stackTop, &POINTERTRANS, 0, SDEBUG); \
    if (tranState == EXCEPTION_CAPTURED ){ \
        FUN_EXC(POINTERTRANS); \
        g_free(POINTERTRANS->stack_backup); \
        g_free(POINTERTRANS->sDebug); \
        g_free(POINTERTRANS); \
        goto ENDTRANS; \
    } else if ( tranState == TRANS_STOP) { \
        FUN_STOP(POINTERTRANS); \
        g_free(POINTERTRANS->stack_backup); \
        g_free(POINTERTRANS->sDebug); \
        g_free(POINTERTRANS); \
        goto ENDTRANS; \
    } else { \
        FUN_START(POINTERTRANS); \
    } 


#define ENDTRANSACTION(POINTERTRANS) \
    ctrans_finish_transaction(POINTERTRANS); \
ENDTRANS: \
    if(TRUE) TRUE; // avoid compiler warnings with empty stuff
    




#endif
