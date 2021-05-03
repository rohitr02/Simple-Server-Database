# CS214-Project3
Pauli Peralta (pp589) and Rohit Rao (rpr79)

Brief description:
This program has a few functions that we needed to test in order to ensure that they were working correctly. Overall we needed to make sure that the program
was correctly reading the bytes given by the client, parsing the bytes received, and ensure that the bytes were formatted correctly. After that, we needed tomake sure that the client received the proper response and that it could handle multiple clients at once. 

Data structure:
We used a linked list to keep track of every (key,value) pair. A linked list provided us with easy access to data and we found that it provided an
intuitive solution to the functionality required by the program.

Parsing and handling client input:
The approach we took to read the input from the client was a byte-by-byte approach. We parsed each input and essetianlly tokenized them and made sure
that each input was formatted correctly, which in this case means that it must have ended with a newline byte. We tested this by giving it illegal 
inputs that were formatted incorrectly and made sure that our program caught them.
	
The first set of instruction was easy to check because every possible legal input consisted of a total of 4 bytes, so anything that was not 4 bytes
had to be incorrect. If 4 bytes were given as input then we just compared the bytes to the possible legal sequence of bytes and checked for a match.
This was tested by providing instructions that did not match "SET,""GET," OR "DEL."
	
The second input needed to be a number that represented the number of incoming bytes. For this we simply checked to see if the input was a valid 
sequence of digits and that it was a positive number.
	
The third input was the key value, the key needed to be a string ending in a newline. The only important part of this input for error checking was
recording its byte size. If the initial requests made were GET or DEL, then the byte side of  the key needed to
match those given by the second input. We recorded the second input, so after tokenizing the third input, we would compare the value of the 
second input to the byte size of the third input and handled it accordingly depending on if they matched or not. We tested this by giving 
mismatches between the value of the second input and the actual byte size of the third input and also by giving it correct cases.

The fourth input only occurred when the request made was for SET. Again, we recorded the number of expected bytes provided by the second input
and used it to compare it to the byte size of the key + the byte size of the value and checked to see if both values matched up. We tested this by
providing keys and values that did not have a byte sum that equaled the second input and cases where they did match. 

After ensuring that each input was correctly handled by the program, we needed to make sure that our data structure was correctly keeping track of the keys and values provided by the client(s), that  each request "GET,""SET," and "DEL" was interacting with the data structure accordingly, and that error messages were printed accordingly.

-"SET":
	After every SET request we printed our data structure on the server side to to see if the keys and values were kept correctly. We would test this
	by providing a combination of new keys and old keys that were to be updated. We would continously provide these sort of test cases and ensure that
	the printed data structure was representative of the set of values we were expecting to see. After every SET, the client receive a "OKS" message to
	notify them that their request was executed.

	Our appraich always stored the key string in a variable that was malloced and the same happed with the value string whenever the request was for 
	SET. If the key already existed, we only freed the key variable that was used to check if the key was already in the data structure. If the key was
	not in the data structure we would just add the key and value and pass ownership to the data structure, which was freed at the end.

-"DEL": 
	Similarly to SET, we continously printed our data structure after each DEL request to ensure that the data structure was updated correctly. DEL 
	requests were tested by providing DEL request with keys that were and were not in the data strcuture. When a key was in the data structure, the
	client receives a "OKD" message to notify them that the key has been unset and deleted followed by the byte size of its value and the value itself.
	If the provided key was not found, then the client receives a "KNF" message to notify them that the key was not found. We found that after each
	DEL request our data structure printed the correct data and thus believed it to be deleting keys correctly.

	Our appraoch always stored the key string in a variable that was malloced, so we made sure to free the key value alone if the key was not present 
	and we made sure to free both the key and value string if they key was indeed present.

-"GET":
	GET was tested by providing the SET request with words that were and were not in the data structure. After confirming that SET and DEL were working
	correctly, we were confident that the data structure should be storing the correct key,value set at all times and simply traversed the data 
	structure in search for the provided key in the GET request. This was tested by providing GET requests with keys that were and were not present in
	the data structure and validating the responses that we were getting, as we know what the responses were supposed to be.	 

	For GET we only used a malloced key variable to store the key and used it to compare, so at the end of the request this was th eonly thing that
	was freed, since GET did not delete nor add anything to the data structure.
