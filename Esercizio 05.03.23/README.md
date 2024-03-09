What you will find in the file:

- I did not make a .h file and just made a unique .c becase the assignment required to turn in a single .c file. I now saw the comment you made on that classroom post, but i already made all of this soooooo yeah.

- The struct Persona has not been altered.
- The struct Database has now a new type of Index node in it that allows quasi-polymorphism.
- In order to accomplish quasi-polymorphism, a set of new datatypes has been created
  - an enum called DataTypes, that contains a voice for each possible data type used (INTEGER and STRING)
  - a union called Dato, that accepts either a int or a char*.
  - a struct called DataStructure that has both a DataType and a Dato. In my code, this is the main way used to interact with record's values and allows for polymorphism
- The old IndexNodes have been replaced with my IndexNode to allow for polymorphism
  - Of course, it still has left and right node because yeah, binary trees, that's kinda the point isn't it
  - The type of value it stores has been changed to a DataStructure
  - It has a reference to a PersonaNode pointer, a linked list where each node has a Persona value.
    - This allows to have more Persona record associated with a single IndexNode
- The insert function has been implemented in order to allow
- I created a unique "Search" function called findByValue instead of the 4 provided. Those 4 will now just call findByValue, so they work fine even if called as-is.
  - findValue will return a PersonaNode pointer. From there, one can fetch all of the Persona records.
  - findByName and company will, instead, directly return the first Persona record from the first node in the PersonaNode linked list. I have done so in order to maintain what little compatibility remained at this point lel.
- The function tasked of freeing all assigned memory is "freeDatabase".
