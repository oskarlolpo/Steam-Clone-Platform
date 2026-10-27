#ifndef POTION_H // If the preprocessor POTION_H is not defined
#define POTION_H // Define the preprocessor POTION_H

#include <string> // Include string library

#include "../drivers/Consumable.cc" // Include consumable class

class Potion : public Consumable
{
private:
    // Attributes
    std::string subtype = "Potion";

public:
    // Constructors
    Potion();
    Potion(std::string name, std::string description, std::string rarity, float weight, int price, int health, int mana, int stamina);

    // Getters

    // Setters

    // Methods
    std::string toString();
};

#endif // End of the preprocessor POTION_H