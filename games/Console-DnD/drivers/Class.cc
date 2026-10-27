#ifndef CLASS_CC // If the file is not defined
#define CLASS_CC // Define the file

#include "../headers/Class.h" // Include the header file

// Constructors

/**
 * @brief Construct a new Class:: Class object
 *
 */

Class::Class()
{
    name = "Warrior";
    description = "All warriors share a masterful command of weapons and armor, and an exhaustive knowledge of combat skills. In addition, they are closely associated with death, both dealing it out and staring it down in defiance.";
    initialWeapon = Weapon();
    initialArmor = Armor();
}

/**
 * @brief Construct a new Class:: Class object
 *
 * @param name
 * @param description
 * @param initialWeapon
 * @param initialArmor
 */

Class::Class(std::string name, std::string description, Weapon initialWeapon, Armor initialArmor)
{
    this->name = name;
    this->description = description;
    this->initialWeapon = initialWeapon;
    this->initialArmor = initialArmor;
}

// Getters

/**
 * @brief Returns the name of the class
 *
 * @return std::string
 */

std::string Class::getName()
{
    return name;
}

/**
 * @brief Returns the description of the class
 *
 * @return std::string
 */

std::string Class::getDescription()
{
    return description;
}

/**
 * @brief Returns the initial weapon of the class
 *
 * @return Weapon
 */

Weapon Class::getInitialWeapon()
{
    return initialWeapon;
}

/**
 * @brief Returns the initial armor of the class
 *
 * @return Armor
 */

Armor Class::getInitialArmor()
{
    return initialArmor;
}

// Methods

/**
 * @brief Returns a string with the information of the class
 *
 * @return std::string
 */

std::string Class::toString()
{
    std::stringstream ss;

    ss << "Класс: " << name << std::endl;
    ss << "Описание: " << description << std::endl;
    ss << std::endl;
    ss << "Стартовое оружие: " << std::endl;
    ss << initialWeapon.toString() << std::endl;
    ss << "Стартовые доспехи: " << std::endl;
    ss << initialArmor.toString() << std::endl;

    return ss.str();
}

#endif // End of the preprocessor CLASS_CC