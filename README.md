	### Recurse Center pair programming: Two-way chat program 
	-----------------------------------------------------
	#### Simple two-way chat application using UDP socket programming 

	````
	Terminal 1: ./chat <port-1>				
	Terminal 2: ./chat <port-2> <port-1>	
	````

	Terminal 1 binds to port-1 and terminal 2 binds to port-2. 
	Anything typed into terminal 2's window will be sent to terminal 1 [100 char max]. 
	On receiving the chat, terminal 1 saves terminal 2's port and it can message terminal 2. 
	Ctrl+C to exit

	#### Example execution: Run `make` to compile 
	````
	$ ./chat 2020 8080
	$ >>> hey guy! 
	$ >>> [8080]: hey pal
	$ >>> [8080]: great to hear from you
	$ >>> <3
	$ >>> 
   
	$ ./chat 8080
	$ >>> [2020]: hey guy!
	$ >>> hey pal 
	$ >>> great to hear from you
	$ >>> [2020]: <3
	$ >>> 
	````
