#ifndef COMBATENGINE_CC
#define COMBATENGINE_CC

#include "../headers/CombatEngine.h"
#include "Functions.cc"
#include "Initializer.cc"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

int CombatEngine::rollD20()
{
    return 1 + (std::rand() % 20);
}

int CombatEngine::rollDice(int count, int sides, int bonus)
{
    int total = bonus;
    for (int i = 0; i < count; ++i)
    {
        total += 1 + (std::rand() % sides);
    }
    return std::max(1, total);
}

int CombatEngine::calculatePlayerAC(Character& character)
{
    // AC = 10 + (DEX modifier) + Armor defense
    int dexMod = (character.getStats().getDexterity() - 10) / 2;
    int armorDef = character.getEquippedArmor().getDefense();
    return std::max(10, 10 + dexMod + armorDef);
}

int CombatEngine::calculatePlayerAttackBonus(Character& character)
{
    // Attack bonus = (STR or DEX modifier) + class level bonus
    int strMod = (character.getStats().getStrength() - 10) / 2;
    int dexMod = (character.getStats().getDexterity() - 10) / 2;
    int bestMod = std::max(strMod, dexMod);
    int proficiency = 2 + (character.getLevel() - 1) / 4;
    return std::max(1, bestMod + proficiency);
}

int CombatEngine::calculatePlayerDamage(Character& character, bool isCrit)
{
    int strMod = (character.getStats().getStrength() - 10) / 2;
    int baseDmg = character.getEquippedWeapon().getDamage();
    if (baseDmg <= 0) baseDmg = 4; // Unarmed strike / default

    // Dice damage: weapon damage is treated as die size
    int roll = 1 + (std::rand() % baseDmg);
    if (isCrit)
    {
        roll += 1 + (std::rand() % baseDmg); // Double dice on critical
    }
    return std::max(1, roll + strMod);
}

void CombatEngine::displayCombatHUD(Character& character, const Monster& enemy, int round)
{
    cleanScreen();
    std::cout << "==================================================================" << std::endl;
    std::cout << "                      ПОШАГОВЫЙ БОЙ  [РАУНД " << round << "]       " << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << std::endl;

    // Enemy visual and HP
    std::cout << "ПРОТИВНИК: " << enemy.getName() << " (" << enemy.getType() << ")" << std::endl;
    std::cout << enemy.getAsciiArt() << std::endl;
    std::cout << "Здоровье врага: " << enemy.getHealthBar() << std::endl;
    std::cout << "Класс доспеха (AC): " << enemy.getArmorClass() << " | Атака: " << enemy.getAttackName() << std::endl;
    std::cout << "------------------------------------------------------------------" << std::endl;

    // Player HP, MP, AC, Weapon
    int playerAC = calculatePlayerAC(character);
    std::cout << "ГЕРОЙ: " << character.getName() << " [" << character.getClass().getName() << ", Ур. " << character.getLevel() << "]" << std::endl;
    std::cout << "Здоровье (HP): " << character.getHealth() << " / " << character.getMaxHealth()
              << " | Мана (MP): " << character.getMana() << " / " << character.getMaxMana()
              << " | Класс доспеха (AC): " << playerAC << std::endl;
    std::cout << "Оружие: " << character.getEquippedWeapon().getName()
              << " (Урон: " << character.getEquippedWeapon().getDamage() << ")"
              << " | Броня: " << character.getEquippedArmor().getName()
              << " (Защита: " << character.getEquippedArmor().getDefense() << ")" << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << std::endl;
}

bool CombatEngine::startBattle(Character& character, Monster enemy)
{
    int round = 1;
    bool playerFled = false;

    // Initiative Roll (D20 + DEX)
    int playerDexMod = (character.getStats().getDexterity() - 10) / 2;
    int playerInit = rollD20() + playerDexMod;
    int enemyInit = rollD20();

    cleanScreen();
    std::cout << "⚔️  НАЧАЛО БИТВЫ!  ⚔️" << std::endl;
    std::cout << "Враг приближается: " << enemy.getName() << "!" << std::endl;
    std::cout << std::endl;
    std::cout << "Бросок инициативы (d20 + ЛОВ):" << std::endl;
    std::cout << "  Ваша инициатива: " << playerInit << " (с бонусом ловкости " << playerDexMod << ")" << std::endl;
    std::cout << "  Инициатива врага: " << enemyInit << std::endl;
    if (playerInit >= enemyInit)
    {
        std::cout << ">>> Вы действуете первым! <<<" << std::endl;
    }
    else
    {
        std::cout << ">>> Противник перехватил инициативу и атакует первым! <<<" << std::endl;
    }
    pause();

    bool playerTurn = (playerInit >= enemyInit);

    while (character.getHealth() > 0 && enemy.isAlive() && !playerFled)
    {
        displayCombatHUD(character, enemy, round);

        if (playerTurn)
        {
            // Player's Turn
            std::cout << "--- ВАШ ХОД ---" << std::endl;
            std::cout << "1. ⚔️  Атаковать оружием" << std::endl;
            std::cout << "2. ✨  Боевое заклинание" << std::endl;
            std::cout << "3. 🧪  Выпить зелье из инвентаря" << std::endl;
            std::cout << "4. 🏃  Попытаться сбежать" << std::endl;
            std::cout << std::endl;
            std::cout << "Выберите действие (1-4): ";

            int action = 0;
            if (!(std::cin >> action))
            {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                action = 1;
            }
            std::cout << std::endl;

            switch (action)
            {
            case 1: // Weapon Attack
            {
                int d20 = rollD20();
                int attackBonus = calculatePlayerAttackBonus(character);
                int totalAttack = d20 + attackBonus;

                std::cout << "🎲 Бросок атаки d20: [" << d20 << "] + бонус " << attackBonus << " = " << totalAttack
                          << " против Класса Доспеха (AC) врага " << enemy.getArmorClass() << std::endl;

                if (d20 == 20)
                {
                    std::cout << "🔥 КРИТИЧЕСКИЙ УДАР! Натуральная 20! Урон удваивается!" << std::endl;
                    int dmg = calculatePlayerDamage(character, true);
                    enemy.takeDamage(dmg);
                    std::cout << "💥 Вы нанесли чудовищный сокрушительный урон: " << dmg << " единиц!" << std::endl;
                }
                else if (d20 == 1)
                {
                    std::cout << "❌ КРИТИЧЕСКИЙ ПРОМАХ! Натуральная 1! Оружие соскользнуло в воздухе!" << std::endl;
                }
                else if (totalAttack >= enemy.getArmorClass())
                {
                    int dmg = calculatePlayerDamage(character, false);
                    enemy.takeDamage(dmg);
                    std::cout << "🗡️ ПОПАДАНИЕ! Вы нанесли " << dmg << " урона врагу " << enemy.getName() << "!" << std::endl;
                }
                else
                {
                    std::cout << "🛡️ ПРОМАХ! Броня врага отразила ваш удар!" << std::endl;
                }
                break;
            }
            case 2: // Spellcast
            {
                std::cout << "Выберите заклинание:" << std::endl;
                std::cout << "1. Огненный снаряд (Расход: 10 MP, Урон: 2d8 огнем)" << std::endl;
                std::cout << "2. Магическая стрела (Расход: 15 MP, Урон: 3d4+3, гарантированное попадание)" << std::endl;
                std::cout << "3. Исцеление ран (Расход: 15 MP, Восстанавливает: 2d8+4 HP)" << std::endl;
                std::cout << "0. Отмена" << std::endl;
                std::cout << "Ваш выбор: ";

                int spellChoice = 0;
                std::cin >> spellChoice;

                if (spellChoice == 1)
                {
                    if (character.getMana() < 10)
                    {
                        std::cout << "Недостаточно маны! (Требуется 10 MP). Ход пропущен." << std::endl;
                    }
                    else
                    {
                        character.setMana(character.getMana() - 10);
                        int d20 = rollD20();
                        int intMod = (character.getStats().getIntelligence() - 10) / 2;
                        if (d20 + intMod + 2 >= enemy.getArmorClass() || d20 == 20)
                        {
                            int dmg = rollDice(2, 8, 2);
                            enemy.takeDamage(dmg);
                            std::cout << "🔥 Огненный снаряд попал точно в цель! Нанесено " << dmg << " урона огнем!" << std::endl;
                        }
                        else
                        {
                            std::cout << "💨 Снаряд пролетел мимо противника!" << std::endl;
                        }
                    }
                }
                else if (spellChoice == 2)
                {
                    if (character.getMana() < 15)
                    {
                        std::cout << "Недостаточно маны! (Требуется 15 MP). Ход пропущен." << std::endl;
                    }
                    else
                    {
                        character.setMana(character.getMana() - 15);
                        int dmg = rollDice(3, 4, 3);
                        enemy.takeDamage(dmg);
                        std::cout << "✨ Самонаводящиеся магические стрелы пронзают врага насквозь! Нанесено " << dmg << " урона силой!" << std::endl;
                    }
                }
                else if (spellChoice == 3)
                {
                    if (character.getMana() < 15)
                    {
                        std::cout << "Недостаточно маны! (Требуется 15 MP). Ход пропущен." << std::endl;
                    }
                    else
                    {
                        character.setMana(character.getMana() - 15);
                        int heal = rollDice(2, 8, 4);
                        character.setHealth(std::min(character.getMaxHealth(), character.getHealth() + heal));
                        std::cout << "💚 Целительная волна света окутывает вас! Восстановлено " << heal << " HP!" << std::endl;
                    }
                }
                else
                {
                    std::cout << "Вы замешкались с выбором заклинания..." << std::endl;
                }
                break;
            }
            case 3: // Drink Potion
            {
                std::vector<Potion> potions = character.getPotions();
                if (potions.empty())
                {
                    std::cout << "В вашем инвентаре нет зелий! Ход пропущен." << std::endl;
                }
                else
                {
                    std::cout << "Ваши зелья:" << std::endl;
                    for (std::size_t i = 0; i < potions.size(); ++i)
                    {
                        std::cout << (i + 1) << ". " << potions[i].getName()
                                  << " (Восстанавливает " << potions[i].getHealth() << " HP, "
                                  << potions[i].getMana() << " MP)" << std::endl;
                    }
                    std::cout << "Выберите номер зелья (или 0 для отмены): ";
                    int pChoice = 0;
                    std::cin >> pChoice;
                    if (pChoice > 0 && pChoice <= static_cast<int>(potions.size()))
                    {
                        Potion chosen = potions[pChoice - 1];
                        character.drinkPotion(pChoice - 1);
                        std::cout << "🧪 Вы выпили " << chosen.getName() << " прямо посреди битвы!" << std::endl;
                    }
                    else
                    {
                        std::cout << "Вы убрали зелье обратно." << std::endl;
                    }
                }
                break;
            }
            case 4: // Flee
            {
                int fleeRoll = rollD20() + playerDexMod;
                std::cout << "🏃 Проверка побега (d20 + ЛОВ): Выпало [" << fleeRoll << "] против Сложности (DC) 12..." << std::endl;
                if (fleeRoll >= 12)
                {
                    std::cout << "💨 Вы ловко уклонились от врага и успешно сбежали из боя!" << std::endl;
                    playerFled = true;
                }
                else
                {
                    std::cout << "❌ Враг перекрыл вам путь к отступлению! Побег провалился!" << std::endl;
                }
                break;
            }
            default:
                std::cout << "Неверное действие. Раунд потерян." << std::endl;
                break;
            }

            pause();
            playerTurn = false;
        }
        else
        {
            // Enemy's Turn
            if (enemy.isAlive())
            {
                std::cout << "--- ХОД ПРОТИВНИКА ---" << std::endl;
                std::cout << enemy.getName() << " проводит атаку: " << enemy.getAttackName() << "!" << std::endl;

                int enemyD20 = rollD20();
                int enemyAttackTotal = enemyD20 + enemy.getAttackBonus();
                int playerAC = calculatePlayerAC(character);

                std::cout << "🎲 Бросок атаки врага: [" << enemyD20 << "] + бонус " << enemy.getAttackBonus()
                          << " = " << enemyAttackTotal << " против вашего Класса Доспеха (AC) " << playerAC << std::endl;

                if (enemyD20 == 20)
                {
                    std::cout << "⚠️ КРИТИЧЕСКИЙ УДАР ВРАГА! Натуральная 20!" << std::endl;
                    int dmg = enemy.rollDamage() * 2;
                    character.setHealth(std::max(0, character.getHealth() - dmg));
                    std::cout << "💥 Вы получили сокрушительный урон: " << dmg << " единиц!" << std::endl;
                }
                else if (enemyD20 == 1)
                {
                    std::cout << "🛡️ Враг споткнулся и нелепо промахнулся!" << std::endl;
                }
                else if (enemyAttackTotal >= playerAC)
                {
                    int dmg = enemy.rollDamage();
                    character.setHealth(std::max(0, character.getHealth() - dmg));
                    std::cout << "🩸 Удар пробил вашу защиту! Получено " << dmg << " урона!" << std::endl;
                }
                else
                {
                    std::cout << "🛡️ Вы успешно заблокировали удар своим щитом и доспехами!" << std::endl;
                }

                pause();
            }

            playerTurn = true;
            round++;
        }
    }

    cleanScreen();

    if (playerFled)
    {
        std::cout << "==================================================================" << std::endl;
        std::cout << "                      ВЫ СБЕЖАЛИ С ПОЛЯ БОЯ!                      " << std::endl;
        std::cout << "==================================================================" << std::endl;
        std::cout << "Вы сохранили жизнь, но не получили опыта и золота." << std::endl;
        pause();
        return false;
    }

    if (!enemy.isAlive())
    {
        int goldWon = enemy.calculateGoldReward();
        int xpWon = enemy.getXpReward();

        character.addGold(goldWon);
        bool leveledUp = character.addExperience(xpWon);

        std::cout << "==================================================================" << std::endl;
        std::cout << "                     🎉  ПОБЕДА В БИТВЕ!  🎉                      " << std::endl;
        std::cout << "==================================================================" << std::endl;
        std::cout << "Вы одолели грозного противника: " << enemy.getName() << "!" << std::endl;
        std::cout << std::endl;
        std::cout << "НАГРАДЫ:" << std::endl;
        std::cout << "  💰 Золото: +" << goldWon << " монет (Всего: " << character.getGold() << ")" << std::endl;
        std::cout << "  ⭐ Опыт: +" << xpWon << " XP (" << character.getExperience() << " / " << character.getNextLevelExperience() << ")" << std::endl;
        std::cout << std::endl;

        if (leveledUp)
        {
            std::cout << "******************************************************************" << std::endl;
            std::cout << "               ✨  НОВЫЙ УРОВЕНЬ! (LEVEL UP!)  ✨                " << std::endl;
            std::cout << "******************************************************************" << std::endl;
            std::cout << "Поздравляем! Ваш уровень вырос до " << character.getLevel() << "!" << std::endl;
            std::cout << "• Максимальное здоровье увеличено! (HP полностью восстановлено: " << character.getMaxHealth() << ")" << std::endl;
            std::cout << "• Максимальная мана увеличена! (MP полностью восстановлено: " << character.getMaxMana() << ")" << std::endl;
            std::cout << "• Все базовые характеристики увеличены на +1!" << std::endl;
            std::cout << "******************************************************************" << std::endl;
        }

        pause();
        return true;
    }

    if (character.getHealth() <= 0)
    {
        std::cout << "==================================================================" << std::endl;
        std::cout << "                       💀  ВЫ ПАЛИ В БОЮ  💀                       " << std::endl;
        std::cout << "==================================================================" << std::endl;
        std::cout << "Противник " << enemy.getName() << " нанёс вам смертельный удар..." << std::endl;
        std::cout << std::endl;
        std::cout << "Торговец Боб подобрал ваше израненное тело и перевязал раны." << std::endl;
        std::cout << "Вы очнулись в таверне с 1 HP и потеряли часть золота на лечение." << std::endl;

        character.setHealth(1);
        int lostGold = character.getGold() / 3;
        character.setGold(character.getGold() - lostGold);

        pause();
        return false;
    }

    return false;
}

void CombatEngine::arenaMenu(Character& character)
{
    if (character.getName() == "None")
    {
        std::cout << "Сначала создайте персонажа или загрузите сохранение!" << std::endl;
        pause();
        return;
    }

    std::vector<Monster> monsters = initMonsters();
    if (monsters.empty())
    {
        std::cout << "Бестиарий пуст! Проверьте файл data/monsters.json." << std::endl;
        pause();
        return;
    }

    int choice = -1;
    do
    {
        cleanScreen();
        std::cout << "==================================================================" << std::endl;
        std::cout << "                  ⚔️  БОЕВАЯ АРЕНА D&D 5E  ⚔️                     " << std::endl;
        std::cout << "==================================================================" << std::endl;
        std::cout << "Добро пожаловать на арену, " << character.getName() << "! [Ур. " << character.getLevel() << "]" << std::endl;
        std::cout << "Здесь вы можете бросить вызов монстрам в пошаговой дуэли." << std::endl;
        std::cout << std::endl;

        for (std::size_t i = 0; i < monsters.size(); ++i)
        {
            std::cout << (i + 1) << ". " << monsters[i].getName()
                      << " [HP: " << monsters[i].getMaxHealth()
                      << " | AC: " << monsters[i].getArmorClass()
                      << " | XP: " << monsters[i].getXpReward() << "]" << std::endl;
        }
        std::cout << std::endl;
        std::cout << "0. Вернуться в главное меню" << std::endl;
        std::cout << std::endl;
        std::cout << "Выберите противника для дуэли (1-" << monsters.size() << "): ";

        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            continue;
        }

        if (choice > 0 && choice <= static_cast<int>(monsters.size()))
        {
            Monster target = monsters[choice - 1];
            startBattle(character, target);
        }
    } while (choice != 0);
}

void CombatEngine::dungeonCrawl(Character& character)
{
    if (character.getName() == "None")
    {
        std::cout << "Сначала создайте персонажа или загрузите сохранение!" << std::endl;
        pause();
        return;
    }

    std::vector<Monster> allMonsters = initMonsters();
    if (allMonsters.size() < 3)
    {
        std::cout << "Недостаточно монстров в бестиарии!" << std::endl;
        pause();
        return;
    }

    cleanScreen();
    std::cout << "==================================================================" << std::endl;
    std::cout << "              🏰  ПОХОД В ЗАБРОШЕННОЕ ПОДЗЕМЕЛЬЕ  🏰               " << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << "Вы спускаетесь по замшелым каменным ступеням в мрачные глубины..." << std::endl;
    std::cout << "Впереди 3 яруса подземелья. В самом конце ждёт Древний Босс!" << std::endl;
    std::cout << "Готовы ли вы рискнуть? (1 - Вперёд / 0 - Назад): ";

    int proceed = 0;
    std::cin >> proceed;
    if (proceed != 1) return;

    // Room 1: Goblin or Skeleton
    Monster r1Monster = (std::rand() % 2 == 0) ? allMonsters[0] : allMonsters[1];
    std::cout << "\n>>> ЭТАЖ 1: Темный склеп <<<" << std::endl;
    std::cout << "Из тьмы выпрыгивает " << r1Monster.getName() << "!" << std::endl;
    pause();

    if (!startBattle(character, r1Monster))
    {
        std::cout << "\nПодземелье оказалось вам не по зубам..." << std::endl;
        pause();
        return;
    }

    // Chest after Room 1
    cleanScreen();
    std::cout << "🎁 После боя вы находите старый кованый сундук!" << std::endl;
    int lootGold = 25 + (std::rand() % 35);
    character.addGold(lootGold);
    std::cout << "Внутри сундука: " << lootGold << " золотых монет и Зелье исцеления!" << std::endl;
    Potion healPot("Зелье среднего лечения", "Восстанавливает 35 HP", "Common", 0.5f, 20, 35, 0, 0);
    character.addPotionToInventory(healPot);
    pause();

    // Short Rest Option
    cleanScreen();
    std::cout << "🏕️ Вы разбили лагерь в тихом гроте. Хотите сделать Короткий отдых?" << std::endl;
    std::cout << "1. Отдохнуть (Восстанавливает 20 HP и 20 MP)" << std::endl;
    std::cout << "2. Идти дальше без отдыха" << std::endl;
    std::cout << "Ваш выбор: ";
    int restChoice = 0;
    std::cin >> restChoice;
    if (restChoice == 1)
    {
        character.setHealth(std::min(character.getMaxHealth(), character.getHealth() + 20));
        character.setMana(std::min(character.getMaxMana(), character.getMana() + 20));
        std::cout << "Силы частично восстановились!" << std::endl;
        pause();
    }

    // Room 2: Orc or Cultist
    Monster r2Monster = (std::rand() % 2 == 0) ? allMonsters[2] : allMonsters[3];
    std::cout << "\n>>> ЭТАЖ 2: Зал ритуалов <<<" << std::endl;
    std::cout << "Воздух вибрирует от темной магии. Навстречу шагает " << r2Monster.getName() << "!" << std::endl;
    pause();

    if (!startBattle(character, r2Monster))
    {
        std::cout << "\nВы пали на втором ярусе..." << std::endl;
        pause();
        return;
    }

    // Room 3: Boss fight (Red Dragon or Necromancer)
    Monster bossMonster = allMonsters.back(); // Last monster is Dragon/Boss
    std::cout << "\n>>> ЭТАЖ 3: ТРОННЫЙ ЗАЛ ДРАКОНА <<<" << std::endl;
    std::cout << "Земля содрогается от поступи чудовища... Перед вами предстает " << bossMonster.getName() << "!" << std::endl;
    pause();

    if (startBattle(character, bossMonster))
    {
        cleanScreen();
        std::cout << "==================================================================" << std::endl;
        std::cout << "            👑  ЛЕГЕНДАРНЫЙ ТРИУМФ! ПОДЗЕМЕЛЬЕ ЗАЧИЩЕНО!  👑       " << std::endl;
        std::cout << "==================================================================" << std::endl;
        std::cout << "Вы одолели величайшее зло и спасли королевство от ужаса!" << std::endl;
        std::cout << "В сокровищнице дракона вы находите легендарный артефакт:" << std::endl;
        std::cout << "🗡️ Меч Пылающего Солнца (+25 урона)!" << std::endl;
        Weapon legendarySword("Меч Пылающего Солнца", "Клинок, выкованный в сердце кометы", "Legendary", 3.0f, 500, "Martial", 25);
        character.addWeaponToInventory(legendarySword);
        pause();
    }
}

#endif // COMBATENGINE_CC
