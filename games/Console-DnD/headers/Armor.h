#ifndef ARMOR_H // If the ARMOR_H preprocessor is not defined
#define ARMOR_H // Define the ARMOR_H preprocessor

#include "../drivers/Item.cc" // Include the object class

class Armor : public Item
{
private:
    // Attributes
    std::string type = "Armor";
    int defense;

public:
    // Constructors
    Armor();
    Armor(std::string name, std::string description, std::string rarity, float weight, int price, std::string subtype, int defense);

    // Getters
    int getDefense();

    // Setters
    void setDefense(int defense);

    // Methods
    std::string toString();
};

#endif // End of the ARMOR_H preprocessor