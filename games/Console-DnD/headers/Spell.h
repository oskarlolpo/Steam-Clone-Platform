#ifndef SPELL_H // If the preprocessor SPELL_H is not defined
#define SPELL_H // Define the preprocessor SPELL_H

#include <iostream> // Library for input and output
#include <string>   // Library for strings
#include <sstream>  // Library for string streams
#include <fstream>  // Library for file streams
#include <cstdlib>  // Library for system functions
#include <vector>   // Library for vectors

#include "../drivers/Functions.cc" // Library for functions

// Spell class

class Spell
{
private:
    // Variables
    std::string name;
    std::string description;
    std::string type;
    int level;
    int damage;
    int manaCost;

public:
    // Constructors
    Spell();
    Spell(std::string name, std::string description, std::string type, int level, int damage, int manaCost);

    // Getters
    std::string getName();
    std::string getDescription();
    std::string getType();
    int getLevel();
    int getDamage();
    int getManaCost();

    // Setters
    void setName(std::string name);
    void setDescription(std::string description);
    void setType(std::string type);
    void setLevel(int level);
    void setDamage(int damage);
    void setManaCost(int manaCost);

    // Methods
    std::string toString();
    void levelUp();
};

#endif // End of the preprocessor SPELL_H