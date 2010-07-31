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

/** @file test1.c
 *
 * @brief Simple example file to show libctrans ussage in practice
 *
 * main starts a transaction and calls indirectly 
 * the function "subSubAction(Transaction* pTrans)" following
 * the stack path main ->  action2 -> subSubAction.
 * subSubAction throws an exception that rollbacks to the
 * start of the transaction in main freeing all the allocated
 * memory.
 * 
 * If the code doesn't raises any exception 
 * it would reach the end of transaction (ctrans_finish_transaction)
 * that will free all allocated resources associated with it.
 */
#include <glib-2.0/glib.h>

#include "libctrans.h"

void action1(  Transaction* pTrans);
void action2(  Transaction* pTrans);
void subAction(Transaction* pTrans);
void subSubAction(Transaction* pTrans);


void exception_captured(Transaction* pTrans);
void transaction_start (Transaction* pTrans);
void transaction_stop  (Transaction* pTrans);

void 
subSubAction(Transaction* pTrans) 
{
    gpointer gp = ctrans_try_malloc(pTrans, 1000, TRUE) ;
    recipEx_type type = USER;
    ctrans_raise_recipient_exception(pTrans,type, "description", "detail", "solution");
}

void 
subAction(Transaction* pTrans) 
{
    gpointer gp = ctrans_try_malloc(pTrans, 1000, TRUE) ;
    subSubAction(pTrans) ;
}

void action1(Transaction* pTrans) 
{
    gpointer gp = ctrans_try_malloc(pTrans, 1000, TRUE) ;
}

void action2(Transaction* pTrans) 
{
    gpointer gp = ctrans_try_malloc(pTrans, 1000, TRUE) ;
    subAction(pTrans);
}

int
main(int nargs, char** args) 
{
                    Transaction* Trans1;
                    NEWTRANSACTION(Trans1, transaction_start, transaction_stop, exception_captured,"Trans1");
    action1(Trans1);
    action2(Trans1);
                    ENDTRANSACTION(Trans1);
}

void 
exception_captured(Transaction* pTrans)
{
    printf("\n********     Exception Captured. Exiting app   ********\n");
    exit(1);
}

void 
transaction_start(Transaction* pTrans)
{
    printf("\n********         TRANS_START                   ********\n");
}

void 
transaction_stop(Transaction* pTrans)
{
    printf("\n********         TRANS_STOP                    ********\n");
    exit(0);
}
