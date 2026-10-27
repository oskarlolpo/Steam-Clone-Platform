#ifndef RACE_CC // If the file is not defined
#define RACE_CC // Define the file

#include "../headers/Race.h" // Include the header file

// Constructors

/**
 * @brief Construct a new Race:: Race object
 *
 */

Race::Race()
{
    name = "Human";
    description = "Humans are the most adaptable and ambitious people among the common races. They have widely varying tastes, morals, and customs in the many different lands where they have settled. When they settle, though, they stay: they build cities to last for the ages, and great kingdoms that can persist for long centuries. An individual human might have a relatively short life span, but a human nation or culture preserves traditions with origins far beyond the reach of any single human’s memory.";
    strengthModifier = 1;
    dexterityModifier = 1;
    constitutionModifier = 1;
    intelligenceModifier = 1;
    wisdomModifier = 1;
    charismaModifier = 1;
}

/**
 * @brief Construct a new Race:: Race object
 *
 * @param name
 * @param description
 * @param strengthModifier
 * @param dexterityModifier
 * @param constitutionModifier
 * @param intelligenceModifier
 * @param wisdomModifier
 * @param charismaModifier
 */

Race::Race(std::string name, std::string description, int strengthModifier, int dexterityModifier, int constitutionModifier, int intelligenceModifier, int wisdomModifier, int charismaModifier)
{
    this->name = name;
    this->description = description;
    this->strengthModifier = strengthModifier;
    this->dexterityModifier = dexterityModifier;
    this->constitutionModifier = constitutionModifier;
    this->intelligenceModifier = intelligenceModifier;
    this->wisdomModifier = wisdomModifier;
    this->charismaModifier = charismaModifier;
}

// Getters

/**
 * @brief Get the Name attribute
 *
 * @return std::string
 */

std::string Race::getName()
{
    return name;
}

/**
 * @brief Get the Description attribute
 *
 * @return std::string
 */

std::string Race::getDescription()
{
    return description;
}

/**
 * @brief Get the Strength Modifier attribute
 *
 * @return int
 */

int Race::getStrengthModifier()
{
    return strengthModifier;
}

/**
 * @brief Get the Dexterity Modifier attribute
 *
 * @return int
 */

int Race::getDexterityModifier()
{
    return dexterityModifier;
}

/**
 * @brief Get the Constitution Modifier attribute
 *
 * @return int
 */

int Race::getConstitutionModifier()
{
    return constitutionModifier;
}

/**
 * @brief Get the Intelligence Modifier attribute
 *
 * @return int
 */

int Race::getIntelligenceModifier()
{
    return intelligenceModifier;
}

/**
 * @brief Get the Wisdom Modifier attribute
 *
 * @return int
 */

int Race::getWisdomModifier()
{
    return wisdomModifier;
}

/**
 * @brief Get the Charisma Modifier attribute
 *
 * @return int
 */

int Race::getCharismaModifier()
{
    return charismaModifier;
}

// Methods

std::string Race::toString()
{
    std::stringstream ss;

    ss << "Раса: " << name << std::endl;
    ss << "Описание: " << description << std::endl;
    ss << "Модификатор силы: " << (strengthModifier >= 0 ? "+" : "") << strengthModifier << std::endl;
    ss << "Модификатор ловкости: " << (dexterityModifier >= 0 ? "+" : "") << dexterityModifier << std::endl;
    ss << "Модификатор телосложения: " << (constitutionModifier >= 0 ? "+" : "") << constitutionModifier << std::endl;
    ss << "Модификатор интеллекта: " << (intelligenceModifier >= 0 ? "+" : "") << intelligenceModifier << std::endl;
    ss << "Модификатор мудрости: " << (wisdomModifier >= 0 ? "+" : "") << wisdomModifier << std::endl;
    ss << "Модификатор харизмы: " << (charismaModifier >= 0 ? "+" : "") << charismaModifier << std::endl;

    return ss.str();
}

#endif // End of the preprocessor RACE_CC