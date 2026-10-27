#ifndef MONSTER_CC
#define MONSTER_CC

#include "../headers/Monster.h"
#include <random>
#include <algorithm>
#include <iomanip>

Monster::Monster()
{
    id = 0;
    name = "Неизвестное существо";
    description = "Неопознанный монстр";
    type = "Неизвестно";
    health = 10;
    maxHealth = 10;
    armorClass = 10;
    attackBonus = 0;
    damageDiceCount = 1;
    damageDiceSides = 4;
    damageBonus = 0;
    attackName = "Простая атака";
    xpReward = 10;
    goldRewardMin = 1;
    goldRewardMax = 5;
    asciiArt = " [?]";
}

Monster::Monster(int id, std::string name, std::string description, std::string type,
                 int health, int maxHealth, int armorClass, int attackBonus,
                 int damageDiceCount, int damageDiceSides, int damageBonus,
                 std::string attackName, int xpReward, int goldRewardMin, int goldRewardMax,
                 std::string asciiArt)
    : id(id), name(name), description(description), type(type),
      health(health), maxHealth(maxHealth), armorClass(armorClass), attackBonus(attackBonus),
      damageDiceCount(damageDiceCount), damageDiceSides(damageDiceSides), damageBonus(damageBonus),
      attackName(attackName), xpReward(xpReward), goldRewardMin(goldRewardMin), goldRewardMax(goldRewardMax),
      asciiArt(asciiArt)
{
}

int Monster::getId() const { return id; }
std::string Monster::getName() const { return name; }
std::string Monster::getDescription() const { return description; }
std::string Monster::getType() const { return type; }
int Monster::getHealth() const { return health; }
int Monster::getMaxHealth() const { return maxHealth; }
int Monster::getArmorClass() const { return armorClass; }
int Monster::getAttackBonus() const { return attackBonus; }
int Monster::getDamageDiceCount() const { return damageDiceCount; }
int Monster::getDamageDiceSides() const { return damageDiceSides; }
int Monster::getDamageBonus() const { return damageBonus; }
std::string Monster::getAttackName() const { return attackName; }
int Monster::getXpReward() const { return xpReward; }
int Monster::getGoldRewardMin() const { return goldRewardMin; }
int Monster::getGoldRewardMax() const { return goldRewardMax; }
std::string Monster::getAsciiArt() const { return asciiArt; }

void Monster::setHealth(int h)
{
    health = std::max(0, std::min(h, maxHealth));
}

bool Monster::isAlive() const
{
    return health > 0;
}

void Monster::takeDamage(int dmg)
{
    health = std::max(0, health - dmg);
}

int Monster::rollAttack() const
{
    // D20 + attack bonus
    int d20 = 1 + (std::rand() % 20);
    return d20 + attackBonus;
}

int Monster::rollDamage() const
{
    int total = damageBonus;
    for (int i = 0; i < damageDiceCount; ++i)
    {
        total += 1 + (std::rand() % damageDiceSides);
    }
    return std::max(1, total);
}

int Monster::calculateGoldReward() const
{
    if (goldRewardMax <= goldRewardMin) return goldRewardMin;
    return goldRewardMin + (std::rand() % (goldRewardMax - goldRewardMin + 1));
}

std::string Monster::getHealthBar() const
{
    const int barWidth = 15;
    float ratio = (maxHealth > 0) ? (static_cast<float>(health) / static_cast<float>(maxHealth)) : 0.0f;
    int filled = static_cast<int>(ratio * barWidth);
    filled = std::max(0, std::min(filled, barWidth));

    std::ostringstream ss;
    ss << "[";
    for (int i = 0; i < filled; ++i) ss << "#";
    for (int i = filled; i < barWidth; ++i) ss << "-";
    ss << "] " << health << "/" << maxHealth << " HP";
    return ss.str();
}

std::string Monster::toString() const
{
    std::ostringstream ss;
    ss << "=== " << name << " ===" << std::endl;
    ss << "Тип: " << type << std::endl;
    ss << "Здоровье (HP): " << health << " / " << maxHealth << std::endl;
    ss << "Класс доспеха (AC): " << armorClass << std::endl;
    ss << "Бонус к атаке: +" << attackBonus << std::endl;
    ss << "Атака: " << attackName << " (" << damageDiceCount << "d" << damageDiceSides << " + " << damageBonus << ")" << std::endl;
    ss << "Награда за победу: " << xpReward << " XP, " << goldRewardMin << "-" << goldRewardMax << " золота" << std::endl;
    ss << "Описание: " << description << std::endl;
    return ss.str();
}

#endif // MONSTER_CC
