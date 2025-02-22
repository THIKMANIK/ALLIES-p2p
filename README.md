Peer-to-Peer Chat Program
Team Name: ALLIES

Team Members:
	•	230001077,Thikmanik(Captain & Leader)
	•	230001022, Darpan
	•	230005052, Praneetha
	•	230001028 , Rishita

Bonus Question: YES, WE HAVE ANSWERED THE BONUS QUESTION. Our program implements a connect() function that allows peers to establish connections with active peers. When a peer connects to another peer, the connection is reflected in the peer table, and the connected peer is displayed when querying active peers.

How to Run the Program Using VS Code
Prerequisites
	•	Operating System: Windows (tested on Windows 10/11).
	•	VS Code
	•	C++ Compiler: Install latest MinGW for Windows:
	•	Winsock Library: The program uses Winsock for networking, which is included in Windows.

Step-by-Step Instructions
1. Set Up VS Code
	•	Open VS Code.

2. Save the Code
	•	Create a new file in VS Code.
	•	Copy and paste the provided C++ code into the file.
	•	Save the file as p2p.cpp in a folder of your choice (e.g., C:\P2P_Chat).


3. Open Multiple Terminals
	•	Open VS Code.
	•	Open multiple terminals:
	◦	Navigate to Terminal > New Terminal in the menu.
	◦	Open as many terminals as needed (one for the server, others for clients).

4. Compile the Code
	•	In each terminal, navigate to the folder where p2p.cpp is saved: cd C:\P2P_Chat

	•	Compile the code using the following command: g++ -o p2p.exe p2p.cpp -lws2_32

	•	This will generate an executable file named p2p.exe.

5. Run the Program
Start the Server
	•	In the first terminal, run the server: ./p2p.exe

	•	Choose option 1 to start the server: Choose an option: 1

	•	The server will start listening on TCP port 8080 and UDP port 9090.

Start the Clients
	•	In other terminals, run the client: ./p2p.exe

	•	Choose option 2 to start the client: Choose an option: 2

	•	Follow the prompts to enter your name and port number: Enter your name: peer1

	•	Enter your port number: 5000

	•	Repeat this step for each client you want to run.


6. Interact with the Program
Send Messages
  •	Use the menu to send messages between peers. Choose an option: 1
	•	Enter recipient IP:PORT: 127.0.0.1:6000
	•	Enter message: Hello peer2!


Query Active Peers
	•	Use the menu to query active peers. Choose an option: 2
	•	[UDP Peer List]:
	•	+---------------------+------------+
	•	|      IP:PORT       | Team Name  |
	•	+---------------------+------------+
	•	| 127.0.0.1:5000     | peer1      |
	•	| 127.0.0.1:6000     | peer2      |
	•	+---------------------+------------+
	

Connect to Active Peers
	•	Use the menu to connect to active peers. Choose an option: 3
	•	Enter peer address (IP:PORT): 127.0.0.1:6000
	•	Connected to peer: 127.0.0.1:6000
	•	Quit
	•	Use the menu to quit the program. Choose an option: 0
	•	Closing connection...


Notes
	•	Ensure that the mandatory IP (10.206.5.228:6555) is reachable. If not, the program will log an error but continue running.
	•	The program uses fixed ports for clients to avoid ephemeral port issues.
	•	If you encounter port conflicts, choose a different port for the client.

Troubleshooting
Connection Issues
	•	Ensure that the server and clients are running on the same network.
	•	Check your firewall settings to allow the program to communicate over the network.
-> Make sure you have the latest mingw installed.

Port Conflicts
	•	If a port is already in use, choose a different port for the client.
Winsock Errors
	•	Ensure that Winsock is properly initialized in the code.

Thank you.
