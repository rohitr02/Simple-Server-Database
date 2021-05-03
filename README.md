# CS214-Project3
## Pauli Peralta (pp589) and Rohit Rao (rpr79)

### Brief Description:
This program has a few major component that we needed to test in order to ensure that it was working correctly. The three major components were: reading and writing data from and to the client, parsing and responding appropriately to data sent from a client, and being able to handle multiple clients using multiple threads and making sure multithreading was functional.

### Usage
To run this program, provide it with a port number between 5000 and 65,536 as an arguement. To communicate with the server, the user can send "GET" / "SET" / "POST" instructions, followed by a positive integer representing the total byte size of the key and/or value to be stored/retrieved/deleted, and then the actual key and/or value to be stored/retrieved/deleted. Each of the arguements ** must be ** separated by a single newline character. Look at the following example for reference:
	SET			<== First Instruction must be "SET", "GET", or "DEL
	11			<== Second Instruction must be the total number of bytes of the key and/or value (** Including spaces and newlines **)
	Day			<== Third Instruction must be the key to be stored/retrieved/deleted
	Sunday		<== Fourth Instruction must be the associated value to be stored. ** This should only be sent when the first instruction is "SET"

### Data Structure:
We made a threadsafe linked list data structure to keep track of every (key,value) pair. A linked list provided us with easy access to data and we found that it provided an intuitive solution to the functionality required by the program.

### Parsing and Handling Client Input:
** Overall Approach **
The approach we took to read the input from the client was a byte-by-byte approach. We parsed each input and tokenized it to make sure
that it was formatted correctly, which in this program means that every input must end with a newline byte. We tested this by giving it illegal inputs that were formatted incorrectly and made sure that our program caught them and responded appropriately.

** First Instruction **
The first set of instructions was trivial to verify because the only legal input consisted of a total of 4 bytes, so any input that was not 4 bytes had to be incorrect. If 4 bytes were given as input then we compared the bytes to the 3 legal inputs -- "SET", "GET", and "DEL" -- and checked for a match. We tested this by providing input that did not match "SET", "GET", OR "DEL" and made sure the program responded appropriately.

** Second Instruction **
The second instruction needed to be a positive integer that represented the number of incoming bytes for the key and/or value. To verify that the 2nd input was legal, we checked to see if the input was a valid sequence of digits and that it was a positive number.

** Third Instruction **
The third instruction was the key/value. The key needed to be a string ending in a newline. The only important part of this input for error checking was recording its byte size. If the initial requests made were GET or DEL, then the byte size of the key needed to
match the input given by the second instruction. To ensure the third instruction was legal, we compared the value of the second input to the byte size of the third input, returned an error if they did not match. We tested this by providing input that was purposefully larger or smaller than the value of the second instruction.

** Fourth Instruction **
The fourth instruction is only neccessary when the first instruction is "SET". Similar to the third instruction case, we ensured this input was legal by adding its byte size to the byte size of the third instruction (the key) and making sure the sum was equivalent to the second instruction. We tested this by purposefully providing keys and values that did not have a byte size which summed up to be equivalent to the second input.

### Testing Responses to Client Input
After ensuring that each input was correctly handled by the program, we needed to make sure that our data structure was correctly keeping track of the keys and values provided by the client(s), specifically that each request -- "GET", "SET", and "DEL" -- was interacting with the data structure appropriately, and that error messages were printed appropriately.

** "SET" **
After every SET request we printed our data structure on the server side to ensure that the keys and values were stored correctly. We would test this by providing a combination of new keys and old keys that were to be updated. We provided an assortment of test cases to ensure that the resulting output of the data structure was correct. After every "SET", the client receives a "OKS" message to
notify them that their request was executed successfully.

** "DEL" **
After every DEL request, we printed our data structure on the server side to ensure that DEL worked properly and our data structure was updated correctly. We tested this part by providing DEL requests for both existent and nonexistent keys in our data structure and verifying that the output to the client was correct eitherways. 
When the key existed in the data structure, the client receives a "OKD" message to notify them that the key and its associated value has been removed and deleted. We also output the byte size of the value and then the value itself.
If the provided key was not found, then the client receives a "KNF" message to notify them that the key was not found. We ensured that after each DEL request our data structure printed the correct data.

** "GET" **
After every GET request, we printed our data structure on the server side to ensure that GET worked properly. We tested this part by providing GET requests for both existent and nonexistent keys in our data structure, and verifying that the output to the client was correct eitherways.

### Testing Multithreading and Multiple Clients
To ensure that the linked list data structure and the rest of the program behaved appropriately with multiple clients, we tested it by running a server and having multiple terminal windows connect simultaneously. We ran each of the above "GET" / "SET" / "DEL" instructions and varying inputs to ensure that there was no undefined behavior and that the results were accurate to what we expected. 

### Extreme Cases
We found that the server can handle messages upwards of 4096 characters long. We tested messages up to this size and ensured appropriate behavior. In the case of malloc or other rare failures, the program is designed to immediately exit with a failure code. Lastly, we tested this program with Address Sanitizer, Undefined Behavior Sanitizer, and Valgrind to make sure there are no memory leaks or undefined behavior.