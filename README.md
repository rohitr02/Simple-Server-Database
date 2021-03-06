Pauli Peralta and Rohit Rao

### Brief Description:
This program is a multithreaded server database that allows multiple users to connect to the server via a port number and get, set, or delete keys and values from the database.

In terms of testing, there are three major components to check: reading and writing data from and to the client, parsing and responding appropriately to data sent from a client, and being able to handle multiple clients using multiple threads and making sure multithreading is functional.

### Usage:
1.) Navigate to the directory of the program in terminal and create the execuatable file by typing: ```make```

2.) To run this program, provide it with a port number between 5000 and 65,536 as an arguement. To communicate with the server, the user can send "GET" / "SET" / "DEL" instructions, followed by a positive integer representing the total byte size (**including spaces and newlines**) of the key and/or value to be stored/retrieved/deleted, and then the actual key and/or value to be stored/retrieved/deleted. Each of the arguments **must be** separated by a single newline character. Look at the following example for reference:

	SET			<== First Instruction must be "SET", "GET", or "DEL"
	11			<== Second Instruction must be the total number of bytes of the key and/or value (Including spaces and newlines)
	Day			<== Third Instruction must be the key to be stored/retrieved/deleted
	Sunday			<== Fourth Instruction must be the associated value to be stored. This should only be sent when the first instruction is "SET"

### Data Structure:
We made a threadsafe linked list data structure to keep track of every (key,value) pair. A linked list provides us with easy access to the data and we found that it provides an intuitive solution to the functionality required by the program.

### Parsing and Handling Client Input:
**Overall Approach:**\
The approach we took to read the input from the client was a byte-by-byte approach. We parsed each input and tokenized it to make sure
that it was formatted correctly, which in this program means that every input must end with a newline byte. We tested this by giving it illegal inputs that were formatted incorrectly and made sure that our program caught them and responded appropriately.

**First Instruction:**\
The first set of instructions was trivial to verify because the only legal input consisted of a total of 4 bytes, so any input that was not 4 bytes had to be incorrect. If 4 bytes were given as input then we compared the bytes to the 3 legal inputs -- "SET", "GET", and "DEL" -- and checked for a match. We tested this by providing input that did not match "SET", "GET", OR "DEL" and made sure the program responded appropriately.

**Second Instruction:**\
The second instruction needed to be a positive integer that represented the number of incoming bytes for the key and/or value. To verify that the 2nd input was legal, we checked to see if the input was a valid sequence of characters that formed a positive integer.

**Third Instruction:**\
The third instruction was the key. The key needed to be a string ending in a newline. The only important part of this input for error checking was recording its byte size. If the initial requests made were "GET" or "DEL", then the byte size of the key needed to match the input given by the second instruction. To ensure the third instruction was legal, we compared the value of the second input to the byte size of the third input, and returned an error if they did not match. We tested this by providing input that was purposefully larger or smaller than the value of the second instruction.

**Fourth Instruction:**\
The fourth instruction is only neccessary when the first instruction is "SET" and the string for this instruction indicates the value of key given in the third instruction. Similar to the third instruction case, we ensured this input was legal by adding its byte size to the byte size of the third instruction (the key) and making sure the sum was equivalent to the second instruction. We tested this by purposefully providing keys and values that did not have a byte size that summed up to be equivalent to the second input, and then making sure the program handled it appropriately.

### Testing Responses to Client Input:
After ensuring that each input was correctly handled by the program, we needed to make sure that our data structure was correctly keeping track of the keys and values provided by the client(s), specifically that each request -- "GET", "SET", and "DEL" -- was interacting with the data structure appropriately, and that error messages were printed appropriately.

**"SET":**\
After every SET request we printed our data structure on the server side to ensure that the keys and values were stored correctly. We would test this by providing a combination of new keys and old keys that were to be updated. We provided an assortment of test cases to ensure that the resulting output of the data structure was correct. After every "SET", the client receives a "OKS" message if their request was executed successfully. Otherwise, the appropriate error message is printed to the client.

**"DEL":**\
After every DEL request, we printed our data structure on the server side to ensure that DEL worked properly and our data structure was updated correctly. We tested this part by providing DEL requests for both existent and nonexistent keys in our data structure and verified that the output to the client was correct eitherway. 
When the key existed in the data structure, the client receives a "OKD" message to notify them that the key and its associated value has been removed and deleted. We also output the byte size of the value and then the value itself. If the provided key was not found, then the client receives a "KNF" message to notify them that the key was not present in the data structure.

**"GET":**\
After every GET request, we printed our data structure on the server side to ensure that GET worked properly. We tested this part by providing GET requests for both existent and nonexistent keys in our data structure, and verifying that the output to the client was correct eitherways. When the key existed in the data structure, the client receives a "OKG" message to notify them that the retrieval was successful. We also output the byte size of the value and then the value itself. If the provided key was not found, then the client receives a "KNF" message to notify them that the key was not found.

### Testing Multithreading and Multiple Clients:
To ensure that the linked list data structure and the rest of the program behaved appropriately with multiple clients, we tested it by running a server and having multiple terminal windows connect simultaneously. We ran each of the above "GET" / "SET" / "DEL" instructions with varying inputs to ensure that there was no undefined behavior and that the results were accurate to what we expected. 

### Extreme Cases:
We found that the server can handle messages upwards of 4096 characters long. We tested messages up to this size and ensured appropriate behavior. In the case of malloc or other rare failures, the program is designed to immediately exit with EXIT_FAILURE. Lastly, we tested this program with Address Sanitizer, Undefined Behavior Sanitizer, and Valgrind to make sure there are no memory leaks or undefined behavior.

### Note: 
The program is meant to run on the Rutgers iLabs servers and is not guaranteed to work on other systems.
