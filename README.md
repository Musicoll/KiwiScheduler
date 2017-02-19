# KiwiScheduler

The scheduler manages time events, called tasks, in a multi-thread context. This implementation allows to have one tasks consumer with several tasks' producer.  Each producer, usually associated with one thread, owns a unique id that is used at the creation of tasks and that specify the producer to which they will be associated to.
