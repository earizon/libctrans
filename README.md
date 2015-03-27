libctrans consists in a set of macros to allow C code to be orginized in transactions in a way something like similar to 
that used in database programming.

When the transaction is "commited" (ended) or rolled back (exception raised) allocated resources are freed.

Unther the hood, a Transaction pointer must be passed to each function and wrapper functions ared used instead of standard ones. For example try_malloc transform into ctrans_try_malloc. Nevertheless, normal C can be mixed if used with care and is NOT discouraged. Basically it means that any C code inside a transaction that makes not use of ctrans frees any used resource before the transaction ends (either normally or through an excption).

libctrans promote code cleaner and safer since it tags logical transaction starts/stops and get rid of repetitive error checking. 
