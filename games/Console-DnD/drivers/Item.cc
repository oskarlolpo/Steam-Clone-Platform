#ifndef ITEM_CC // If the file is not defined
#define ITEM_CC // Define the file

#include "../headers/Item.h" // Include the header file

// Constructors

/**
 * @brief Construct a new Item:: Item object
 *
 */

Item::Item()
{
    name = "None";
    description = "None";
    rarity = "None";
    weight = 0.0;
    price = 0;
    type = "None";
    subtype = "None";
}

/**
 * @brief Construct a new Item:: Item object
 *
 * @param name
 * @param description
 * @param rarity
 * @param weight
 * @param price
 * @param type
 * @param subtype
 */

Item::Item(std::string name, std::string description, std::string rarity, float weight, int price, std::string type, std::string subtype)
{
    this->name = name;
    this->description = description;
    this->rarity = rarity;
    this->weight = weight;
    this->price = price;
    this->type = type;
    this->subtype = subtype;
}

// Getters

/**
 * @brief Get the Name attribute
 *
 * @return std::string
 */

std::string Item::getName()
{
    return name;
}

/**
 * @brief Get the Description attribute
 *
 * @return std::string
 */

std::string Item::getDescription()
{
    return description;
}
 
/**
 * @brief  Get the Rarity attribute
 * 
 * @return std::string 
 */

std::string Item::getRarity()
{
    return rarity;
}

/**
 * @brief Get the Weight attribute
 *
 * @return float
 */

float Item::getWeight()
{
    return weight;
}

/**
 * @brief Get the Price attribute
 *
 * @return int
 */

int Item::getPrice()
{
    return price;
}

/**
 * @brief Get the Type attribute
 *
 * @return std::string
 */

std::string Item::getType()
{
    return type;
}

/**
 * @brief Get the Subtype attribute
 *
 * @return std::string
 */

std::string Item::getSubtype()
{
    return subtype;
}

// Setters

/**
 * @brief Set the Name attribute
 *
 * @param name
 */

void Item::setName(std::string name)
{
    this->name = name;
}

/**
 * @brief Set the Description attribute
 *
 * @param description
 */

void Item::setDescription(std::string description)
{
    this->description = description;
}

/**
 * @brief Set the Rarity attribute
 *
 * @param rarity
 */

void Item::setRarity(std::string rarity)
{
    this->rarity = rarity;
}

/**
 * @brief Set the Weight attribute
 *
 * @param weight
 */

void Item::setWeight(float weight)
{
    this->weight = weight;
}

/**
 * @brief Set the Price attribute
 *
 * @param value
 */

void Item::setPrice(int price)
{
    this->price = price;
}

/**
 * @brief Set the Type attribute
 *
 * @param type
 */

void Item::setType(std::string type)
{
    this->type = type;
}

/**
 * @brief Set the Subtype attribute
 *
 * @param subtype
 */

void Item::setSubtype(std::string subtype)
{
    this->subtype = subtype;
}

// Methods

std::string Item::toString()
{
    std::stringstream ss;

    ss << "Название: " << name << std::endl;
    ss << "Описание: " << description << std::endl;
    ss << "Редкость: " << rarity << std::endl;
    ss << "Вес: " << weight << std::endl;
    ss << "Цена: " << price << " золота" << std::endl;
    ss << "Тип: " << type << std::endl;
    ss << "Подтип: " << subtype << std::endl;

    return ss.str();
}

#endif // End of the preprocessor ITEM_CC