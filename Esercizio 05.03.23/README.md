What you will find in the file:

- I did not make a .h file and just made a unique .c becase the assignment required to turn in a single .c file. I now saw the comment you made on that classroom post, but i already made all of this soooooo yeah.

- The structs Persona and Database have not been altered.
- In order to accomplish quasi-polymorphism, a set of new datatypes has been created
  - an enum called DataTypes, that contains a voice for each possible data type used (INTEGER and STRING)
  - a union called Dato, that accepts either a int or a char*.
  - a struct called DataStructure that has both a DataType and a Dato. In my code, this is the main way used to interact with record's values and allows for polymorphism
- The old IndexNodes have been replaced with my IndexNode to allow for polymorphism
  - Of course, it still has left and right node because yeah, binary trees, that's kinda the point isn't it
  - The type of value it stores has been changed to a DataStructure
  - It has a reference to a Persona pointer, because why should we waste or O(n) search cost if we can just allocate a bit more memory to make up for it
- The insert function has been implemented without any noteworthy feature, apart from it resembling your worst OOP nightmare
- I created a unique "Search" function called findByValue instead of the 4 provided. Those 4 will now just call findByValue, so they work fine even if called as-is.
- The function tasked of freeing all assigned memory is "freeDatabase".
