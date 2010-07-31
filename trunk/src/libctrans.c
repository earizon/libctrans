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

#include "libctrans.h"

void
ctrans_backup_Stack(guint32* stack_ptr2Top, Transaction* pTrans,guint32 *p2Stk) 
{
    guint32 stackBotton;
    pTrans->stack_ptr2Top = stack_ptr2Top;
    int stack_size           = pTrans->stack_ptr2Top - p2Stk;
    pTrans->stack_backup  = (guint32 *)g_malloc(stack_size*sizeof(guint32));
    pTrans->stack_size    = stack_size;
    int idx;
    for (idx = 0; idx<stack_size; ++idx) 
      {
         pTrans->stack_backup[idx] = pTrans->stack_ptr2Top[-idx];
      }
}

void
ctrans_restore_Stack(Transaction* pTrans)
{ 
    // TODO(1): Implementation depends on CPU architecture. 
    //          Next code work just for x86 (stack growing down in memory).
    guint32 stackBotton;
    int idx=0;
    for (idx = 0; idx<pTrans->stack_size; ++idx) 
      {
        pTrans->stack_ptr2Top[-idx] = pTrans->stack_backup[idx]; 
      }
}


gint
ctrans_new_transaction (guint32* stack_ptr2Top, Transaction** ppTrans, 
    Transaction* pParentTrans, gchar* sDebug) 
{
    guint32 ptr2stackBotton;
    *ppTrans = g_malloc(sizeof(Transaction));
    if (*ppTrans == NULL) 
      {
        g_print("CRITICAL: Unable to allocate memory. Exiting now");
        exit(1);
      }
    ctrans_backup_Stack(stack_ptr2Top, *ppTrans, &ptr2stackBotton);
    (*ppTrans)->allocated_memory   = 0 ;
    (*ppTrans)->child_transactions = 0 ;
    if (pParentTrans != 0) 
      {
        (*pParentTrans).child_transactions = g_list_prepend(
            (*pParentTrans).child_transactions,*ppTrans);
      }
    if (sDebug == NULL) 
      {
        (*ppTrans)->sDebug = g_strdup("");
      }
    else
      {
        (*ppTrans)->sDebug = g_strdup(sDebug);
      }
    int result = setjmp((*ppTrans)->transStart);

    if (result==TRANS_STOP || result==EXCEPTION_CAPTURED) 
      {
        ctrans_free_resources(*ppTrans);
      }
    return result;
}

gpointer
ctrans_try_malloc (Transaction* pTrans, gsize n_bytes, gboolean bRaiseException) 
{
    gpointer result = g_try_malloc(n_bytes);
    if (!result) 
      {
        if (bRaiseException==FALSE) return NULL;
        // If NULL is not allowed raise an administratorException
        // TODO(0): This is not "very legal" for a library, but enough for a first draft.
        // It could be caused by a programming mistake with an n_bytes non-sense value.
        // Change for a senderException.
        ctrans_raise_sender_exception(pTrans, 1/*TODO(0): guint32 type*/, "g_try_malloc failed @ ctrans_try_malloc", "") ;
      }
    pTrans->allocated_memory = g_list_append(
        pTrans->allocated_memory, result);
    return result;
}

void
ctrans_free_resources (Transaction* pTrans) 
{
    if ((*pTrans).child_transactions != 0 ) 
      {
        //TODO(0): ctrans_free_resources over child_transactions
      }
    if ((*pTrans).allocated_memory != 0 ) 
      {
        guint listSize = g_list_length((*pTrans).allocated_memory);
        guint idx=0;
        for (idx=0; idx<listSize; idx++) 
          {
            gpointer p = g_list_nth_data((*pTrans).allocated_memory,idx);
            g_free(p);
          }
        g_list_free((*pTrans).allocated_memory);

      }
}

void 
ctrans_finish_transaction(Transaction* pTrans) 
{
    // TODO(0): Implementation depends on CPU architecture. 
    //          Next code work just for x86 (stack growing down in memory).
    if (  (pTrans->stack_ptr2Top - pTrans->stack_size) <= (guint32*) & pTrans ) 
      {
        guint32 foolArray[100];
        ctrans_finish_transaction(pTrans)  ;
        return  ; // <-- It will never reach this code actually
      }
    ctrans_restore_Stack(pTrans);
    longjmp(pTrans->transStart,TRANS_STOP);
}

void 
ctrans_raise_exception(Transaction* pTrans, exception_base* exception) 
{
    // TODO(0): Implementation depends on CPU architecture. 
    //          Next code work just for x86 (stack growing down in memory).
    if (  (pTrans->stack_ptr2Top - pTrans->stack_size) <= (guint32*) & pTrans ) 
      {
        guint32 foolArray[100];
        ctrans_raise_exception(pTrans, exception) ;
        return  ; // <-- It will never reach this code actually
      }
    // TODO(1): Log exception history?
    ctrans_restore_Stack(pTrans);
    pTrans->raisedException = (exception_base*)exception;
    longjmp(pTrans->transStart,EXCEPTION_CAPTURED);
}

void 
ctrans_raise_sender_exception(Transaction* pTrans,
    guint32 type, char* description_i18n, char* detail_i18n) 
{
    // TODO(0): Free g_malloc
    recipient_exception* re = (recipient_exception *) 
            g_malloc(sizeof(recipient_exception));
    ((exception_base*)re)->type = type;
    ((exception_base*)re)->serial_number        = 0; //TODO(1)
    ((exception_base*)re)->parent_serial_number = 0; //TODO(1)
    ((exception_base*)re)->ms_timestamp         = 0; //TODO(1)
    ((exception_base*)re)->description_i18n     = description_i18n;
    ((exception_base*)re)->detail_i18n          = description_i18n;
    ((exception_base*)re)->gthread              = (GThread*)0; //TODO(1)
    ctrans_raise_exception(pTrans, (exception_base*) re) ;
};

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

void 
ctrans_raise_recipient_exception(Transaction* pTrans,
    recipEx_type type, char* description_i18n, char* detail_i18n, char* solution_i18n) 
{
    // TODO(0): Free g_malloc
    recipient_exception* re = (recipient_exception *) 
            malloc(sizeof(recipient_exception));
    ((exception_base*)re)->type = type;
    ((exception_base*)re)->serial_number        = 0; //TODO(1)
    ((exception_base*)re)->parent_serial_number = 0; //TODO(1)
    ((exception_base*)re)->ms_timestamp         = 0; //TODO(1)
    ((exception_base*)re)->description_i18n     = description_i18n;
    ((exception_base*)re)->detail_i18n          = description_i18n;
    ((exception_base*)re)->gthread              = (GThread*)0; //TODO(1)
    re->solution_i18n                           = solution_i18n;
    ctrans_raise_exception(pTrans, (exception_base*) re) ;
};
