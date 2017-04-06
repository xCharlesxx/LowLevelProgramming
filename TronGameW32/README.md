LLP Exercise 2
--> Code: Headers & Source
--> Documentation: Supporting Docs
--> Libs: SFML
--> Projects: Visual Studio Projects
--> Props: Shared Properties
--> Resources: Assets

Controls: WASD to move, supports controller analog sticks. Space bar to advance and Escape to exit or end the game. 
Known Bugs: Rapidly changing direction sometimes results in the client getting caught in a send loop which slows down the server. 
            Server will has trouble disconnecting clients, once a client leaves the server will cease to respond to any more messages. 
            More than two clients results in some de-syncronisation. 
When you win there is a dramatic victory animation where the winner's colour fills the map, do not be alarmed, it is not crashing, even though it may look like it. 