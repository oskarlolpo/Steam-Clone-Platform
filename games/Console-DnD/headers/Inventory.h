#ifndef INVENTORY_H // If the preprocessor INVENTORY_H is not defined
#define INVENTORY_H // Define the preprocessor INVENTORY_H

#include <vector> // Include vector library

#include "../drivers/Weapon.cc"     // Include the Weapon class
#include "../drivers/Armor.cc"      // Include the Armor class
#include "../drivers/Potion.cc" // Include the Potion class
#include "../drivers/QuestItem.cc"  // Include the QuestItem class

class Inventory
{
private:
    // Attributes
    int gold;
    float maxWeight;
    float currentWeight;
    std::vector<Weapon> weapons;
    std::vector<Armor> armors;
    std::vector<Potion> potions;
    std::vector<QuestItem> questItems;

public:
    // Constructors
    Inventory();
    Inventory(float capacity);

    // Getters
    int getGold();
    float getMaxWeight();
    float getCurrentWeight();
    std::vector<Weapon> getWeapons();
    std::vector<Armor> getArmors();
    std::vector<Potion> getPotions();
    std::vector<QuestItem> getQuestItems();

    // Setters
    void setGold(int gold);
    void setMaxWeight(float maxWeight);
    void setCurrentWeight(float currentWeight);
    void setWeapons(std::vector<Weapon> weapons);
    void setArmors(std::vector<Armor> armors);
    void setPotions(std::vector<Potion> potions);
    void setQuestItems(std::vector<QuestItem> questItems);

    // Methods
    void addWeapon(Weapon weapon);
    void addArmor(Armor armor);
    void addPotion(Potion potion);
    void addQuestItem(QuestItem questItem);

    void removeWeapon(int index);
    void removeArmor(int index);
    void removePotion(int index);
    void removeQuestItem(int index);
};

#endif // End of the preprocessor INVENTORY_H