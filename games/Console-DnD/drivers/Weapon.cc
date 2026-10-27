#ifndef WEAPON_CC // If the file is not defined
#define WEAPON_CC // Define the file

#include "../headers/Weapon.h" // Include the header file

// Constructors

/**
 * @brief Construct a new Weapon:: Weapon object
 *
 */

Weapon::Weapon() : Item()
{
    damage = 0;
}

/**
 * @brief Construct a new Weapon:: Weapon object
 *
 * @param name
 * @param description
 * @param rarity
 * @param weight
 * @param price
 * @param subtype
 * @param damage
 */

Weapon::Weapon(std::string name, std::string description, std::string rarity, float weight, int price, std::string subtype, int damage) : Item(name, description, rarity, weight, price, "Weapon", subtype)
{
    this->damage = damage;
}

// Getters

/**
 * @brief Get the Damage attribute
 *
 * @return int
 */

int Weapon::getDamage()
{
    return damage;
}

// Setters

/**
 * @brief Set the Damage attribute
 *
 * @param damage
 */

void Weapon::setDamage(int damage)
{
    this->damage = damage;
}

// Methods

/**
 * @brief Get the string representation of the object
 *
 * @return std::string
 */

std::string Weapon::toString()
{
    // Create a string stream
    std::stringstream ss;

    // Inherit the Item::toString() method
    std::string item = Item::toString();

    // Add the Item attributes to the string stream
    ss << item;

    // Add the Weapon attributes to the string stream
    ss << "Урон: " << damage << std::endl;
    
    // Return the string stream as a string
    return ss.str();
}

#endif // End of the preprocessor WEAPON_CC