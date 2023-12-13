Sample README.txt

Joanna did the sender part in MS1, and implemented the entirety of the main and server.cpp, along with a lot of debugging

Lucas did the receiver part in MS1, and implemented room.cpp and the message queue

Both of us styled/cleaned up our parts of the code separately.

Note on tests: with regard to the automated tests provided, sequential works fine, however interleaved has a minor error. The
program runs all the relevant tests/commands in the script just fine with the expected output, but fails when cleaning up 
because it tries to remove an empty directory. We dont know exactly why this is happening, nor what it's an indicator of. 
Concurrent also runs with errors. When forking, it declares that resources arent available.


Critical sections are handled in message_queue, room, and server files.
    For the first, it uses two primitives: a semaphore and 
mutex (through guard objects) with the critical sections being in the enqueue/dequeue functions, whenever the message list of
the object is modified. each list of messages is specific to one client, so it must only be accessed by one client, which is
what the mutex is for. The semaphore is essential for communicating between functions, so that the message is given to the
receiver when and only when one or more messages are placed into the queue.
    For room.cpp, it uses mutexes to control the different rooms. Critical sections are whenever the list of receivers is
modified, (adding/removing a user from the room) or when a message is broadcasted, where all of the users' message queues are 
altered. Mutexes/guards are necessary because this should only happen for one room at a time.
    In server.cpp, [add something about creating threads here]. It also uses mutexes/guards when it has to deal with different 
rooms or clients in the last two functions. These areas are critical points because they either handle a specific room, or a 
specific client, which shouldnt be modified by other threads. It also uses a guard when creating another thread.

[include stuff about race conditions and deadlocks here]


Instructions:

Please include where your critical sections are, how you determined them, 
and why you chose the synchronization primitives for each section. 
You should also explain how your critical sections ensure that the 
synchronization requirements are met without introducing synchronization 
hazards (e.g. race conditions and deadlocks).

