Sample README.txt

Joanna did the sender part in MS1, and implemented the entirety of the main and server.cpp, along with a lot of debugging

Lucas did the receiver part in MS1, and implemented room.cpp and the message queue

Both of us styled/cleaned up our parts of the code separately.

Note on tests: with regard to the automated tests provided, sequential works fine, however interleaved has a minor error. The
program runs all the relevant tests/commands in the script just fine with the expected output, but fails when cleaning up 
because it tries to remove an empty directory. We dont know exactly why this is happening, nor what it's an indicator of. 
Concurrent also runs with errors. When forking, it declares that resources arent available.








Please include where your critical sections are, how you determined them, 
and why you chose the synchronization primitives for each section. 
You should also explain how your critical sections ensure that the 
synchronization requirements are met without introducing synchronization 
hazards (e.g. race conditions and deadlocks).

