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

#include <gtk/gtk.h>

#include "libctrans.h"

/** @file test3.c
 *
 * @brief example of GUI app interacting with the transaction system
 *
 * This example is based on
 * http://library.gnome.org/devel/gtk-tutorial/2.17/c39.html#SEC-HELLOWORLD
 * adding transaction aware code.
 * 
 * The three different callbacks are unified into a single callback_control.<br>
 * gpointer data is used to differentiate the origin of the callback.
 * 
 * callback_control then is in charge of starting/ending transactions.
 * A transaction start/stop involves two different GUI events in this example.<br>
 * It's quite natural to associate a transaction to a user dialog (a user
 * filling e few forms in a row).
 */

typedef enum { HELLO, BYE, DELETE, DESTROY } action;

void exception_captured(Transaction* pTrans);
void transaction_start (Transaction* pTrans);
void transaction_stop  (Transaction* pTrans);

GtkWidget *window, *butHello, *butBye;
GtkWidget *box1;

static Transaction* Trans1;
static gboolean 
callback_control( GtkWidget *widget, GdkEvent  *event, gpointer data )
{
    gboolean result = TRUE;
    static gint16 count  = 0;
    if ( (*(action*)data) == DESTROY) 
      {
        gtk_main_quit ();
      } 
    else if ( (*(action*)data) == DELETE) 
      {
        result = TRUE; // FALSE=> GTK will emit a "destroy" signal.
        gtk_main_quit ();
      } 
    else if ( (*(action*)data) == HELLO) 
      {
        NEWTRANSACTION(Trans1, transaction_start, transaction_stop, exception_captured,
           "trans Hello-Bye");
        g_print ("Hello World\n");
      } 
    else if ( (*(action*)data) == BYE) 
      {
         g_print ("Bye World\n");
         count++;
         if (count==10)
           {
             count=0;
             recipEx_type type = USER;
             ctrans_raise_recipient_exception(Trans1,type,
                 "count==10", "detail", "solution");
           }
         ENDTRANSACTION(Trans1);
      } 
    else 
      {
        //TODO(0): Throw implementation_exception
      } 
    return result;
}



void 
transaction_start(Transaction* pTrans)
{
    printf("\n********         TRANS_START                   ********\n");
    gtk_widget_hide (GTK_WIDGET (butHello));
    gtk_widget_show (GTK_WIDGET (butBye));
}

void
transaction_end_common(Transaction* pTrans){
    gtk_widget_show (GTK_WIDGET (butHello));
    gtk_widget_hide (GTK_WIDGET (butBye));
}

void 
transaction_stop(Transaction* pTrans)
{
    transaction_end_common(pTrans);
    printf("\n********         TRANS_STOP                    ********\n");
}

void 
exception_captured(Transaction* pTrans)
{
    transaction_end_common(pTrans);
    printf("\n********     Exception Captured.               ********\n");
    printf("\n********     pTrans->sDebug:%s.   ********\n",pTrans->sDebug);
}

int 
main( int   argc, char *argv[] )
{
    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    box1 = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box1), 10);
    butHello = gtk_button_new_with_label ("Hello World");
    butBye = gtk_button_new_with_label ("Bye Bye");

    action action_HELLO   = HELLO;
    action action_BYE     = BYE;
    action action_DELETE  = DELETE;
    action action_DESTROY = DESTROY;
    g_signal_connect (G_OBJECT (window)  , "delete_event"      , G_CALLBACK (callback_control), (gpointer) & action_DELETE);
    g_signal_connect (G_OBJECT (window)  , "destroy"           , G_CALLBACK (callback_control), (gpointer) & action_DESTROY);
    g_signal_connect (G_OBJECT (butHello), "button_press_event", G_CALLBACK (callback_control), (gpointer) & action_HELLO);
    g_signal_connect (G_OBJECT (butBye)  , "button_press_event", G_CALLBACK (callback_control), (gpointer) & action_BYE);
    gtk_container_add (GTK_CONTAINER (window), box1);
    gtk_box_pack_start (GTK_BOX(box1), butHello, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX (box1), butBye, TRUE, TRUE, 0);

    gtk_widget_show (butHello);
    gtk_widget_show (box1);
    gtk_widget_show (window);
    gtk_main ();
    return 0;
}
