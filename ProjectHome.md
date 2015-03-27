libctrans is not transparent to the user (C programmer). Code has to be changed to take advantage of it. Still its ussage is quite intuitive and the performance is quite similar to normal C without transaction support: Basically it means a Transaction pointer must be passed to each function and wrapper functions used instead of standard ones. For example try\_malloc transform into ctrans\_try\_malloc. Nevertheless, normal C can be mixed if used with care and is NOT discouraged. Basically it means that any C code inside a transaction that makes not use of ctrans frees any used resource before the transaction ends (either normally or through an excption).

libctrans promote code cleaner and safer since it tags logical transaction starts/stops and get rid of repetitive error checking.