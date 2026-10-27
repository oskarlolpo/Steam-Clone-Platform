#ifndef COMBATENGINE_H
#define COMBATENGINE_H

#include "../headers/Character.h"
#include "../headers/Monster.h"
#include <vector>

class CombatEngine
{
public:
    // Dice rolling utilities
    static int rollD20();
    static int rollDice(int count, int sides, int bonus = 0);

    // Single battle against a specific monster
    // Returns true if player won, false if player died or fled
    static bool startBattle(Character& character, Monster enemy);

    // Arena / Battle hub menu
    static void arenaMenu(Character& character);

    // Dungeon Crawl: sequence of encounters with rewards
    static void dungeonCrawl(Character& character);

private:
    static void displayCombatHUD(Character& character, const Monster& enemy, int round);
    static int calculatePlayerAC(Character& character);
    static int calculatePlayerAttackBonus(Character& character);
    static int calculatePlayerDamage(Character& character, bool isCrit);
};

#endif // COMBATENGINE_H
