#ifndef POTION_CC // If the file is not defined
#define POTION_CC // Define the file

#include <iostream> // Include iostream library
#include <string>   // Include string library
#include <sstream>  // Include sstream library

#include "../headers/Potion.h" // Include the header file

// Constructors

/**
 * @brief Construct a new Potion:: Potion object
 *
 */

Potion::Potion() : Consumable()
{
    subtype = "Potion";
}

/**
 * @brief Construct a new Potion:: Potion object
 *
 * @param name
 * @param description
 * @param rarity
 * @param weight
 * @param price
 * @param health
 * @param mana
 * @param stamina
 */

Potion::Potion(std::string name, std::string description, std::string rarity, float weight, int price, int health, int mana, int stamina) : Consumable(name, description, rarity, weight, price, "Potion", health, mana, stamina) {}

// Methods

/**
 * @brief This method returns the object as a string
 *
 * @return std::string
 */

std::string Potion::toString()
{
    // Ihnerit the toString method from the Consumable class
    std::string consumable = Consumable::toString();

    // Return the string
    return consumable;
}

#endif // End of the file