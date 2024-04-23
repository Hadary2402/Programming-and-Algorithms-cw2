# Programming-and-Algorithms-cw2


![README picture](https://github.com/Hadary2402/Programming-and-Algorithms-cw2/assets/114479685/c72eaf60-07c5-4d69-a4bc-893482353586)

Instructions of Program Usage

1.	Needed to run the program:
      a.	Make sure a C++ compiler installed on the system, for example GCC.

      b.	Ensure the installation of OpenSSL installed to handle cryptographic functions.

      c.	Opening multiple command lines or terminals to run the server and then the client(s).

3.	Now to Run the Program:
   
      a.Compile the codes:

         i.	Open a terminal or cmd on windows.

            ii.	Go to the directory that contains the “server.cpp” and “client.cpp” and run the following commands:

            For the server: “g++ -o server server.cpp -lpthread -lcrypto”

            For the client: “g++ -o client client.cpp -std=c++11 -lssl -lcrypto -pthread”

      b.Run the program in one of the terminals after navigating to its directory with the command: “./client”

       c.User interaction:

            i.	After the client runs, the user is prompted with options to sign up, login, or exiting the program.

            ii.	The user can then enter their credentials when signing and then logging in.

            iii.	Now after the user has logged in, the user joins the chat room and it’s announced in the server and can start sending and receiving messages.

       d.Exiting the Program:

            i.	When exiting the client program, the user can simple type “#exit” and press enter.

            ii.	To stop the server from running use CTRL+C when clicked on the terminal, this also works on the client.

Notes:

•	Ensure the server is running before connecting with the client.

•	Ensure that the server and client are on the same network, reachable via network.

•	You can modify the IP address and Port to be able to connect through different networks especially if the server is on a different machine.

•	This code was created and ran on kali linux and OpenSSL Version is 3.1.5-1
