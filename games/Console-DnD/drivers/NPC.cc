#ifndef NPC_CC // If the file is not defined
#define NPC_CC // Define the file

#include "../headers/NPC.h" // Include the NPC header file
#include "Functions.cc"     // Include the functions file

// Constructors

/**
 * @brief Construct a new NPC::NPC object
 * 
 * @param 
 * @return 
 */

NPC::NPC()
{
    this->name = "NPC";
    this->description = "A non-playable character";
}

/**
 * @brief Construct a new NPC::NPC object
 * 
 * @param name 
 * @param description 
 * @return 
 */

NPC::NPC(std::string name, std::string description)
{
    this->name = name;
    this->description = description;
}

// Getters

/**
 * @brief Get the Name object
 * 
 * @param 
 * @return std::string 
 */

std::string NPC::getName()
{
    return this->name;
}

/**
 * @brief Get the Description object
 * 
 * @param 
 * @return std::string 
 */

std::string NPC::getDescription()
{
    return this->description;
}

// Setters

/**
 * @brief Set the Name object
 * 
 * @param name 
 * @return void 
 */

void NPC::setName(std::string name)
{
    this->name = name;
}

/**
 * @brief Set the Description object
 * 
 * @param description 
 * @return void 
 */

void NPC::setDescription(std::string description)
{
    this->description = description;
}

// Methods

/**
 * @brief This method is used to print the NPC information.
 * 
 * @param 
 * @return std::string  
 */

std::string NPC::toString()
{
    std::stringstream ss;

    ss << "Имя: " << name << std::endl;
    ss << "Описание: " << description << std::endl;

    return ss.str();
}

#endif // End of the preprocessor NPC_CC