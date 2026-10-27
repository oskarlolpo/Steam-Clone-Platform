#ifndef QUESTITEM_H // If the QUESTITEM_H preprocessor is not defined
#define QUESTITEM_H // Define the QUESTITEM_H preprocessor

#include "../drivers/Item.cc" // Include the object class

class QuestItem : public Item
{
private:
    // Attributes
    std::string type = "QuestItem";
    std::string quest;

public:
    // Constructors
    QuestItem();
    QuestItem(std::string name, std::string description, std::string rarity, float weight, int price, std::string subtype, std::string quest);

    // Getters
    std::string getQuest();

    // Setters

    // There are no setters for this class because the quest is not going to change.

    // Methods
    std::string toString();
};

#endif // End of the QUESTITEM_H preprocessor