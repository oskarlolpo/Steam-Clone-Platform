#ifndef CONSUMABLE_H // If the preprocessor CONSUMABLE_H is not defined
#define CONSUMABLE_H // Define the preprocessor CONSUMABLE_H

#include "../drivers/Item.cc" // Include the Item class

class Consumable : public Item
{
protected:
    // Attributes
    std::string type = "Consumable";
    int health;
    int mana;
    int stamina;

public:
    // Constructors
    Consumable();
    Consumable(std::string name, std::string description, std::string rarity, float weight, int price, std::string subtype, int health, int mana, int stamina);

    // Getters
    int getHealth();
    int getMana();
    int getStamina();

    // Setters

    // There are no setters because the attributes are constant

    // Methods
    std::string toString();
};

#endif // End of the preprocessor CONSUMABLE_H