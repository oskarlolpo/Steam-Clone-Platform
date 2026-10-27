#ifndef QUESTITEM_CC // If the file is not defined
#define QUESTITEM_CC // Define the file

#include "../headers/QuestItem.h" // Include the header file

// Constructors

/**
 * @brief Construct a new QuestItem:: QuestItem object
 * 
 */

QuestItem::QuestItem() : Item()
{
    quest = "None";
}

/**
 * @brief Construct a new QuestItem:: QuestItem object
 * 
 * @param name 
 * @param description 
 * @param rarity
 * @param weight 
 * @param price 
 * @param subtype  
 * @param quest 
 */

QuestItem::QuestItem(std::string name, std::string description, std::string rarity, float weight, int price, std::string subtype, std::string quest) : Item(name, description, rarity, weight, price, "QuestItem", subtype)
{
    this->quest = quest;
}


// Getters

/**
 * @brief Get the Quest object
 * 
 * @return std::string 
 */

std::string QuestItem::getQuest()
{
    return quest;
}

// Setters

// There are no setters for this class because the quest is not going to change.

// Methods

/**
 * @brief This method returns a string with the information of the object
 * 
 * @return std::string 
 */

std::string QuestItem::toString()
{
    // Create a string stream
    std::stringstream ss;

    // Inherit the Item::toString() method
    std::string item = Item::toString();

    // Add the Item attributes to the string stream
    ss << item;

    // Add the QuestItem attributes to the string stream
    ss << "Квест: " << quest << std::endl;
    
    // Return the string stream as a string
    return ss.str();
}

#endif // End of the preprocessor QUESTITEM_CC