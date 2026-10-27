#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <sstream>

class Monster
{
private:
    int id;
    std::string name;
    std::string description;
    std::string type;

    int health;
    int maxHealth;
    int armorClass;
    int attackBonus;

    int damageDiceCount;
    int damageDiceSides;
    int damageBonus;

    std::string attackName;
    int xpReward;
    int goldRewardMin;
    int goldRewardMax;
    std::string asciiArt;

public:
    Monster();
    Monster(int id, std::string name, std::string description, std::string type,
            int health, int maxHealth, int armorClass, int attackBonus,
            int damageDiceCount, int damageDiceSides, int damageBonus,
            std::string attackName, int xpReward, int goldRewardMin, int goldRewardMax,
            std::string asciiArt);

    // Getters
    int getId() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getType() const;
    int getHealth() const;
    int getMaxHealth() const;
    int getArmorClass() const;
    int getAttackBonus() const;
    int getDamageDiceCount() const;
    int getDamageDiceSides() const;
    int getDamageBonus() const;
    std::string getAttackName() const;
    int getXpReward() const;
    int getGoldRewardMin() const;
    int getGoldRewardMax() const;
    std::string getAsciiArt() const;

    // Setters
    void setHealth(int health);

    // Combat methods
    bool isAlive() const;
    void takeDamage(int damage);
    int rollAttack() const;
    int rollDamage() const;
    int calculateGoldReward() const;

    std::string toString() const;
    std::string getHealthBar() const;
};

#endif // MONSTER_H
