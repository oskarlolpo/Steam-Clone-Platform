#ifndef NPC_H // If the NPC_H preprocessor is not defined
#define NPC_H // Define the NPC_H preprocessor

#include <iostream> // Include input and output library
#include <string>   // Include string library
#include <sstream>  // Include string stream library
#include <cstdlib>  // Include system library
#include <vector>   // Include vector library

class NPC
{
protected:
    // Attributes
    std::string name;
    std::string description;

public:
    // Constructors
    NPC();
    NPC(std::string name, std::string description);

    // Getters
    std::string getName();
    std::string getDescription();

    // Setters
    void setName(std::string name);
    void setDescription(std::string description);

    // Methods
    std::string toString();
};

#endif // End of the NPC_H preprocessor