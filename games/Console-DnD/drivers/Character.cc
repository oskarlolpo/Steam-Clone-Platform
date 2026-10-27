#ifndef CHARACTER_CC // If the file is not defined
#define CHARACTER_CC // Define the file

#include <string>  // Include string library
#include <sstream> // Include string stream library
#include <vector>  // Include vector library

#include "../headers/Character.h" // Include the header file
#include "Initializer.cc"         // Include the Initializer file
#include "Functions.cc"           // Include the Functions file

// Constructors

/**
 * @brief Construct a new Character:: Character object
 *
 */

Character::Character()
{
    name = "None";
    gender = "None";
    age = 0;
    alignment = "True Neutral";

    health = 100;
    mana = 100;
    stamina = 100;

    maxHealth = 100;
    maxMana = 100;
    maxStamina = 100;

    damage = 0;
    defense = 0;

    level = 1;
    experience = 0;
    nextLevelExperience = 100;

    race = Race();
    clss = Class();
    stats = Stats(race);

    equippedWeapon = clss.getInitialWeapon();
    equippedArmor = clss.getInitialArmor();
}

// Getters

/**
 * @brief Get the Name attribute
 *
 * @return std::string
 */

std::string Character::getName()
{
    return name;
}

/**
 * @brief Get the Gender attribute
 *
 * @return std::string
 */

std::string Character::getGender()
{
    return gender;
}

/**
 * @brief Get the Age attribute
 *
 * @return int
 */

int Character::getAge()
{
    return age;
}

/**
 * @brief Get the Alignment attribute
 *
 * @return std::string
 */

std::string Character::getAlignment()
{
    return alignment;
}

/**
 * @brief Get the Health attribute
 *
 * @return int
 */

int Character::getHealth()
{
    return health;
}

/**
 * @brief Get the Mana attribute
 *
 * @return int
 */

int Character::getMana()
{
    return mana;
}

/**
 * @brief Get the Stamina attribute
 *
 * @return int
 */

int Character::getStamina()
{
    return stamina;
}

/**
 * @brief Get the Max Health attribute
 *
 * @return int
 */

int Character::getMaxHealth()
{
    return maxHealth;
}

/**
 * @brief Get the Max Mana attribute
 *
 * @return int
 */

int Character::getMaxMana()
{
    return maxMana;
}

/**
 * @brief Get the Max Stamina attribute
 *
 * @return int
 */

int Character::getMaxStamina()
{
    return maxStamina;
}

/**
 * @brief Get the Damage attribute
 *
 * @return int
 */

int Character::getDamage()
{
    return damage;
}

/**
 * @brief Get the Defense attribute
 *
 * @return int
 */

int Character::getDefense()
{
    return defense;
}

// Setters

/**
 * @brief Set the Name attribute
 *
 * @param name
 */

void Character::setName(std::string name)
{
    this->name = name;
}

/**
 * @brief Set the Gender attribute
 *
 * @param gender
 */

void Character::setGender(std::string gender)
{
    this->gender = gender;
}

/**
 * @brief Set the Age attribute
 *
 * @param age
 */

void Character::setAge(int age)
{
    this->age = age;
}

/**
 * @brief Set the Alignment attribute
 *
 * @param alignment
 */

void Character::setAlignment(std::string alignment)
{
    this->alignment = alignment;
}

/**
 * @brief Set the Health attribute
 *
 * @param health
 */

void Character::setHealth(int health)
{
    this->health = health;
}

/**
 * @brief Set the Mana attribute
 *
 * @param mana
 */

void Character::setMana(int mana)
{
    this->mana = mana;
}

/**
 * @brief Set the Stamina attribute
 *
 * @param stamina
 */

void Character::setStamina(int stamina)
{
    this->stamina = stamina;
}

/**
 * @brief Set the Max Health attribute
 *
 * @param maxHealth
 */

void Character::setMaxHealth(int maxHealth)
{
    this->maxHealth = maxHealth;
}

/**
 * @brief Set the Max Mana attribute
 *
 * @param maxMana
 */

void Character::setMaxMana(int maxMana)
{
    this->maxMana = maxMana;
}

/**
 * @brief Set the Max Stamina attribute
 *
 * @param maxStamina
 */

void Character::setMaxStamina(int maxStamina)
{
    this->maxStamina = maxStamina;
}

/**
 * @brief Set the Damage attribute
 *
 * @param attack
 */

void Character::setDamage(int damage)
{
    this->damage = damage;
}

/**
 * @brief Set the Defense attribute
 *
 * @param defense
 */

void Character::setDefense(int defense)
{
    this->defense = defense;
}

// Methods

/**
 * @brief Function to create a character with the data entered by the user
 *
 * @param character
 * @return
 */

void Character::createCharacter()
{
    // Variables
    std::string name;
    std::string gender;
    int age;
    std::string alignment;
    Race race;
    int option;

    // Vectors
    std::vector<std::string> genders = listGenders();
    std::vector<std::string> alignments = listAlignments();
    std::vector<std::string> races = listRaces();
    std::vector<std::string> classes = listClasses();

    // Clear the screen
    cleanScreen();

    // Введение
    std::cout << "Приветствую, искатель приключений! Добро пожаловать в мир D&D!" << std::endl;
    std::cout << "Сейчас мы создадим вашего уникального персонажа." << std::endl;
    std::cout << "Вы готовы начать?" << std::endl;
    pause();

    // Очистка экрана
    cleanScreen();

    // Имя
    std::cout << "Как зовут вашего героя? ";
    getline(std::cin >> std::ws, name);
    std::cout << std::endl;
    this->name = name;

    cleanScreen();

    // Пол
    std::cout << "Выберите пол персонажа:" << std::endl;
    printStringVector(genders);
    std::cout << "Ваш выбор: ";
    std::cin >> option;
    while (option < 1 || option > (int)genders.size())
    {
        std::cout << "Неверный выбор. Попробуйте снова: ";
        std::cin >> option;
    }
    if (option == 3)
    {
        std::string specificGender;
        std::cout << "Укажите ваш вариант пола: ";
        getline(std::cin >> std::ws, specificGender);
        this->gender = specificGender;
    }
    else
    {
        this->gender = genders[option - 1];
    }
    std::cout << std::endl;

    cleanScreen();

    // Возраст
    std::cout << "Укажите возраст: ";
    std::cin >> age;
    while (age < 0)
    {
        std::cout << "Недопустимый возраст. Введите корректное число: ";
        std::cin >> age;
    }
    std::cout << std::endl;
    this->age = age;

    cleanScreen();

    // Мировоззрение
    std::cout << "Выберите мировоззрение:" << std::endl;
    printStringVector(alignments);
    std::cout << "Ваш выбор: ";
    std::cin >> option;
    while (option < 1 || option > (int)alignments.size())
    {
        std::cout << "Неверный выбор. Попробуйте снова: ";
        std::cin >> option;
    }
    std::cout << std::endl;
    this->alignment = alignments[option - 1];

    cleanScreen();

    // Раса
    std::cout << "Выберите расу:" << std::endl;
    printStringVector(races);
    std::cout << "Ваш выбор: ";
    std::cin >> option;
    while (option < 1 || option > (int)races.size())
    {
        std::cout << "Неверный выбор. Попробуйте снова: ";
        std::cin >> option;
    }
    std::cout << std::endl;
    this->defineRace(option - 1);

    cleanScreen();

    // Класс
    std::cout << "Выберите класс:" << std::endl;
    printStringVector(classes);
    std::cout << "Ваш выбор: ";
    std::cin >> option;
    while (option < 1 || option > (int)classes.size())
    {
        std::cout << "Неверный выбор. Попробуйте снова: ";
        std::cin >> option;
    }
    std::cout << std::endl;
    this->defineClass(option - 1);

    cleanScreen();

    // Характеристики
    std::cout << "Генерация характеристик..." << std::endl;
    this->defineStats();
    pause();

    cleanScreen();

    std::cout << "Поздравляем! Ваш персонаж успешно создан!" << std::endl;
}

/**
 * @brief Get the string representation of the object
 *
 * @return std::string
 */

std::string Character::toString()
{
    std::stringstream ss;

    ss << readAsciiArt("asciiArt/myCharacter.txt") << std::endl;

    ss << "=== ЛИСТ ПЕРСОНАЖА ===" << std::endl;
    ss << "Имя: " << name << std::endl;
    ss << "Уровень: " << level << "  [Опыт: " << experience << " / " << nextLevelExperience << " XP]" << std::endl;
    ss << "Здоровье (HP): " << health << " / " << maxHealth << std::endl;
    ss << "Мана (MP): " << mana << " / " << maxMana << std::endl;
    ss << "Пол: " << gender << std::endl;
    ss << "Возраст: " << age << std::endl;
    ss << "Мировоззрение: " << alignment << std::endl;
    ss << "Раса: " << race.getName() << std::endl;
    ss << "Класс: " << clss.getName() << std::endl;
    ss << "Золото: " << inventory.getGold() << " монет" << std::endl;
    ss << std::endl;

    ss << "=== БОЕВЫЕ ПАРАМЕТРЫ ===" << std::endl;
    ss << "Урон: " << damage << std::endl;
    ss << "Защита: " << defense << std::endl;
    ss << std::endl;

    ss << "=== ХАРАКТЕРИСТИКИ ===" << std::endl;
    ss << stats.toString() << std::endl;
    ss << std::endl;

    ss << "=== ЭКИПИРОВАННОЕ ОРУЖИЕ ===" << std::endl;
    if (equippedWeapon.getName() == "None")
    {
        ss << "Оружие не экипировано." << std::endl;
    }
    else
    {
        ss << equippedWeapon.toString() << std::endl;
    }
    ss << std::endl;

    ss << "=== ЭКИПИРОВАННЫЕ ДОСПЕХИ ===" << std::endl;
    if (equippedArmor.getName() == "None")
    {
        ss << "Доспехи не экипированы." << std::endl;
    }
    else
    {
        ss << equippedArmor.toString() << std::endl;
    }
    ss << std::endl;

    return ss.str();
}

/**
 * @brief Get the gold of the character from the inventory
 *
 * @return int
 */

int Character::getGold()
{
    return inventory.getGold();
}

/**
 * @brief Set the gold of the character
 *
 * @param gold
 * @return
 */

void Character::setGold(int gold)
{
    inventory.setGold(gold);
}

/**
 * @brief Add gold to the character
 *
 * @param gold
 * @return
 */

void Character::addGold(int gold)
{
    inventory.setGold(inventory.getGold() + gold);
}

/**
 * @brief Define the race of the character
 *
 * @param option
 * @return
 */

void Character::defineRace(int option)
{
    // Initialize races vector
    std::vector<Race> races = initRaces();

    // Set race
    race = races[option];
}

/**
 * @brief Define the stats of the character
 *
 * @return
 */

void Character::defineStats()
{
    // Clear the screen
    cleanScreen();  

    // Если персонаж не полуэльф, характеристики генерируются случайно
    if (race.getName() != "Полуэльф" && race.getName() != "Half-Elf")
    {
        stats = Stats(race);
        std::cout << "Базовые характеристики успешно сгенерированы случайным образом." << std::endl;
    }
    else
    {
        int option = 0;
        int stat1 = 0;
        int stat2 = 0;

        std::vector<std::string> statsNames = listStats();

        std::cout << "Вы выбрали расу Полуэльф! Вы можете повысить две любые характеристики на +1." << std::endl;
        std::cout << "Какие характеристики вы желаете улучшить?" << std::endl;
        std::cout << std::endl;

        printStringVector(statsNames);
        std::cout << std::endl;

        std::cout << "Выберите первую характеристику: ";
        std::cin >> option;

        while (option < 1 || option > (int)statsNames.size())
        {
            std::cout << "Неверный вариант. Попробуйте снова: ";
            std::cin >> option;
        }

        std::cout << "Вы выбрали: " << statsNames[option - 1] << std::endl;
        pause();

        stat1 = option;
        cleanScreen();

        std::cout << "Выберите вторую характеристику:" << std::endl;
        std::cout << std::endl;

        printStringVector(statsNames);

        std::cout << "Ваш выбор: ";
        std::cin >> option;

        while (option < 1 || option > (int)statsNames.size() || option == stat1)
        {
            if (option == stat1)
            {
                std::cout << "Эта характеристика уже была выбрана! Выберите другую: ";
            }
            else
            {
                std::cout << "Неверный вариант. Попробуйте снова: ";
            }
            std::cin >> option;
        }

        std::cout << "Вы выбрали: " << statsNames[option - 1] << std::endl;
        pause();

        cleanScreen();

        stat2 = option;

        stats = Stats(race);
        stats.increaseByOne(stat1);
        stats.increaseByOne(stat2);

        std::cout << "Характеристики успешно сохранены." << std::endl;
    }
}

/**
 * @brief Define the class of the character
 *
 * @param option
 * @return
 */

void Character::defineClass(int option)
{
    // Initialize classes vector
    std::vector<Class> classes = initClasses();

    // Set class
    clss = classes[option];

    // Set initial weapon
    equippedWeapon = clss.getInitialWeapon();
    damage += clss.getInitialWeapon().getDamage();

    // Set initial armor
    equippedArmor = clss.getInitialArmor();
    defense += clss.getInitialArmor().getDefense();
}

/**
 * @brief Show the inventory
 *
 */

void Character::showInventory()
{
    // Variables
    int option = 0;
    int itemNum = 1;
    int numWeapons;
    int numArmors;
    int numPotions;
    int numQuestItems;

    do
    {
        // Reset the item number
        itemNum = 1;

        // Get the number of items
        numWeapons = inventory.getWeapons().size();
        numArmors = inventory.getArmors().size();
        numPotions = inventory.getPotions().size();
        numQuestItems = inventory.getQuestItems().size();

        // Clear the screen
        cleanScreen();

        // Title
        std::cout << readAsciiArt("asciiArt/inventory.txt") << std::endl;

        // Золото
        std::cout << "Золото: " << inventory.getGold() << std::endl;
        std::cout << std::endl;

        // Текущий и максимальный вес
        std::cout << "Текущий вес: " << inventory.getCurrentWeight() << std::endl;
        std::cout << "Максимальный вес: " << inventory.getMaxWeight() << std::endl;
        std::cout << std::endl;

        // Оружие
        if (numWeapons > 0)
        {
            std::cout << "Оружие: " << std::endl;
            for (int i = 0; i < numWeapons; i++)
            {
                std::cout << itemNum << ". " << inventory.getWeapons()[i].getName() << std::endl;
                itemNum++;
            }
            std::cout << std::endl;
        }

        // Доспехи
        if (numArmors > 0)
        {
            std::cout << "Доспехи: " << std::endl;
            for (int i = 0; i < numArmors; i++)
            {
                std::cout << itemNum << ". " << inventory.getArmors()[i].getName() << std::endl;
                itemNum++;
            }
            std::cout << std::endl;
        }

        // Зелья
        if (numPotions > 0)
        {
            std::cout << "Зелья: " << std::endl;
            for (int i = 0; i < numPotions; i++)
            {
                std::cout << itemNum << ". " << inventory.getPotions()[i].getName() << std::endl;
                itemNum++;
            }
            std::cout << std::endl;
        }

        // Квестовые предметы
        if (numQuestItems > 0)
        {
            std::cout << "Квестовые предметы: " << std::endl;
            for (int i = 0; i < numQuestItems; i++)
            {
                std::cout << itemNum << ". " << inventory.getQuestItems()[i].getName() << std::endl;
                itemNum++;
            }
            std::cout << std::endl;
        }

        // Выход
        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите предмет: ";
        std::cin >> option;
        std::cout << std::endl;

        if (option != 0)
        {
            cleanScreen();

            int itemOption = 0;

            if (option <= numWeapons)
            {
                std::cout << inventory.getWeapons()[option - 1].toString() << std::endl;
            }
            else if (option <= numWeapons + numArmors)
            {
                std::cout << inventory.getArmors()[option - numWeapons - 1].toString() << std::endl;
            }
            else if (option <= numWeapons + numArmors + numPotions)
            {
                std::cout << inventory.getPotions()[option - numWeapons - numArmors - 1].toString() << std::endl;
            }
            else if (option <= numWeapons + numArmors + numPotions + numQuestItems)
            {
                std::cout << inventory.getQuestItems()[option - numWeapons - numArmors - numPotions - 1].toString() << std::endl;
            }

            std::cout << "1. Использовать / Экипировать" << std::endl;
            std::cout << "2. Выбросить" << std::endl;
            std::cout << "3. Назад" << std::endl;
            std::cout << std::endl;
            std::cout << "Выберите действие: ";
            std::cin >> itemOption;
            std::cout << std::endl;

            cleanScreen();

            switch (itemOption)
            {
            case 1:
                if (option <= numWeapons)
                {
                    equipWeapon(option - 1);
                }
                else if (option <= numWeapons + numArmors)
                {
                    equipArmor(option - numWeapons - 1);
                }
                else if (option <= numWeapons + numArmors + numPotions)
                {
                    drinkPotion(option - numWeapons - numArmors - 1);
                }
                else if (option <= numWeapons + numArmors + numPotions + numQuestItems)
                {
                    std::cout << "Квестовые предметы нельзя использовать." << std::endl;
                }
                pause();
                break;
            case 2:
                if (option <= (int)inventory.getWeapons().size())
                {
                    inventory.removeWeapon(option - 1);
                    std::cout << "Оружие выброшено из инвентаря." << std::endl;
                }
                else if (option <= (int)inventory.getWeapons().size() + numArmors)
                {
                    inventory.removeArmor(option - inventory.getWeapons().size() - 1);
                    std::cout << "Доспех выброшен из инвентаря." << std::endl;
                }
                else if (option <= (int)inventory.getWeapons().size() + numArmors + numPotions)
                {
                    inventory.removePotion(option - inventory.getWeapons().size() - numArmors - 1);
                    std::cout << "Зелье выброшено из инвентаря." << std::endl;
                }
                else if (option <= (int)inventory.getWeapons().size() + numArmors + numPotions + numQuestItems)
                {
                    inventory.removeQuestItem(option - inventory.getWeapons().size() - numArmors - numPotions - 1);
                    std::cout << "Квестовый предмет выброшен." << std::endl;
                }
                pause();
                break;
            case 3:
                break;
            default:
                break;
            }
        }

    } while (option != 0);
}

/**
 * @brief Method to show the equipped items
 *
 */

void Character::showEquippedItems()
{
    // Variables
    int option;
    int itemOption;
    int itemNum = 1;
    bool weaponEquipped = false;
    bool armorEquipped = false;

    do
    {
        // Clear the screen
        cleanScreen();

        // Reset the variables
        itemNum = 1;
        weaponEquipped = false;
        armorEquipped = false;

        // Check if the weapon is equipped
        if (equippedWeapon.getName() != "None")
        {
            weaponEquipped = true;
        }

        // Check if the armor is equipped
        if (equippedArmor.getName() != "None")
        {
            armorEquipped = true;
        }

        // Title
        std::cout << readAsciiArt("asciiArt/equipment.txt") << std::endl;

        // Оружие
        std::cout << "Оружие: " << std::endl;
        if (weaponEquipped)
        {
            std::cout << itemNum << ". " << equippedWeapon.getName() << std::endl;
            itemNum++;
        }
        else
        {
            std::cout << "Оружие не экипировано." << std::endl;
        }
        std::cout << std::endl;

        // Броня
        std::cout << "Доспехи: " << std::endl;
        if (armorEquipped)
        {
            std::cout << itemNum << ". " << equippedArmor.getName() << std::endl;
            itemNum++;
        }
        else
        {
            std::cout << "Доспехи не экипированы." << std::endl;
        }
        std::cout << std::endl;

        // Назад
        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите предмет: ";
        std::cin >> option;

        if (option == 0)
        {
            return;
        }
        else
        {
            cleanScreen();

            if (option == 1 && weaponEquipped)
            {
                std::cout << "=== ЭКИПИРОВАННОЕ ОРУЖИЕ ===" << std::endl;
                std::cout << equippedWeapon.toString() << std::endl;
                std::cout << std::endl;

                std::cout << "1. Снять" << std::endl;
                std::cout << "2. Назад" << std::endl;

                std::cout << "Выберите действие: ";
                std::cin >> itemOption;

                if (itemOption == 1)
                {
                    cleanScreen();
                    unequipWeapon();
                    pause();
                }
                else if (itemOption == 2)
                {
                    return;
                }
            }
            else if ((option == 2 && weaponEquipped && armorEquipped) || (option == 1 && !weaponEquipped && armorEquipped))
            {
                std::cout << "=== ЭКИПИРОВАННЫЕ ДОСПЕХИ ===" << std::endl;
                std::cout << equippedArmor.toString() << std::endl;
                std::cout << std::endl;

                std::cout << "1. Снять" << std::endl;
                std::cout << "2. Назад" << std::endl;

                std::cout << "Выберите действие: ";
                std::cin >> itemOption;

                if (itemOption == 1)
                {
                    cleanScreen();
                    unequipArmor();
                    pause();
                }
                else if (itemOption == 2)
                {
                    return;
                }
            }
        }

    } while (option != 0);
}

/**
 * @brief Get the character's weapons in the inventory
 *
 * @return std::vector<Weapon>
 */

std::vector<Weapon> Character::getWeapons()
{
    return inventory.getWeapons();
}

/**
 * @brief Get the character's armors in the inventory
 *
 * @return std::vector<Armor>
 */

std::vector<Armor> Character::getArmors()
{
    return inventory.getArmors();
}

/**
 * @brief Get the character's potions in the inventory
 *
 * @return std::vector<Potion>
 */

std::vector<Potion> Character::getPotions()
{
    return inventory.getPotions();
}

/**
 * @brief Get the character's quest items in the inventory
 *
 * @return std::vector<QuestItem>
 */

std::vector<QuestItem> Character::getQuestItems()
{
    return inventory.getQuestItems();
}

/**
 * @brief Method to add a weapon to the inventory
 *
 * @param weapon
 */

void Character::addWeaponToInventory(Weapon weapon)
{
    inventory.addWeapon(weapon);
}

/**
 * @brief Method to add an armor to the inventory
 *
 * @param armor
 */

void Character::addArmorToInventory(Armor armor)
{
    inventory.addArmor(armor);
}

/**
 * @brief Method to add a potion to the inventory
 *
 * @param potion
 */

void Character::addPotionToInventory(Potion potion)
{
    inventory.addPotion(potion);
}

/**
 * @brief Method to add a quest item to the inventory
 *
 * @param questItem
 */

void Character::addQuestItemToInventory(QuestItem questItem)
{
    inventory.addQuestItem(questItem);
}

/**
 * @brief Method to remove a weapon from the inventory
 *
 * @param index
 */

void Character::removeWeaponFromInventory(int index)
{
    inventory.removeWeapon(index);
}

/**
 * @brief Method to remove an armor from the inventory
 *
 * @param index
 */

void Character::removeArmorFromInventory(int index)
{
    inventory.removeArmor(index);
}

/**
 * @brief Method to remove a potion from the inventory
 *
 * @param index
 */

void Character::removePotionFromInventory(int index)
{
    inventory.removePotion(index);
}

/**
 * @brief Method to remove a quest item from the inventory
 *
 * @param index
 */

void Character::removeQuestItemFromInventory(int index)
{
    inventory.removeQuestItem(index);
}

/**
 * @brief Method to equip a weapon
 *
 * @param index
 */

void Character::equipWeapon(int index)
{
    // Variables
    int option;

    // Check if the weapon is equipped
    if (equippedWeapon.getName() != "None")
    {
        std::cout << "Оружие уже экипировано." << std::endl;
        std::cout << "Хотите снять текущее оружие перед сменой?" << std::endl;
        std::cout << "1. Да" << std::endl;
        std::cout << "2. Нет" << std::endl;
        std::cout << "Ваш выбор: ";
        std::cin >> option;

        if (option == 1)
        {
            unequipWeapon();
        }
    }
    else
    {
        option = 1;
    }

    if (option == 1)
    {
        equippedWeapon = inventory.getWeapons()[index];
        damage += equippedWeapon.getDamage();
        inventory.removeWeapon(index);
        std::cout << "Оружие успешно экипировано." << std::endl;
    }
}

/**
 * @brief Method to equip an armor
 *
 * @param index
 */

void Character::equipArmor(int index)
{
    // Variables
    int option;

    // Check if the armor is equipped
    if (equippedArmor.getName() != "None")
    {
        std::cout << "Доспехи уже надеты." << std::endl;
        std::cout << "Хотите снять текущие доспехи перед сменой?" << std::endl;
        std::cout << "1. Да" << std::endl;
        std::cout << "2. Нет" << std::endl;
        std::cout << "Ваш выбор: ";
        std::cin >> option;

        if (option == 1)
        {
            unequipArmor();
        }
    }
    else
    {
        option = 1;
    }

    if (option == 1)
    {
        equippedArmor = inventory.getArmors()[index];
        defense += equippedArmor.getDefense();
        inventory.removeArmor(index);
        std::cout << "Доспехи успешно надеты." << std::endl;
    }
}

/**
 * @brief Method to drink a potion
 *
 * @param index
 */

void Character::drinkPotion(int index)
{
    // Check if the potion adds health
    if (inventory.getPotions()[index].getHealth() > 0)
    {
        // Check if the character is at full health
        if (health == maxHealth)
        {
            std::cout << "У вас уже максимальный запас здоровья." << std::endl;
        }
        else
        {
            // Check if the character's health will be over the max health
            if (health + inventory.getPotions()[index].getHealth() > maxHealth)
            {
                // Set the character's health to the max health
                health = maxHealth;
            }
            else
            {
                // Add the health to the character's health
                health += inventory.getPotions()[index].getHealth();
            }

            std::cout << "Вы выпили " << inventory.getPotions()[index].getName() << "." << std::endl;
            std::cout << "Ваше здоровье теперь: " << health << "." << std::endl;

            // Remove the potion from the inventory
            inventory.removePotion(index);
        }
        // Check if the potion adds mana
        if (inventory.getPotions()[index].getMana() > 0)
        {
            // Check if the character is at full mana
            if (mana == maxMana)
            {
                std::cout << "У вас уже максимальный запас маны." << std::endl;
            }
            else
            {
                // Check if the character's mana will be over the max mana
                if (mana + inventory.getPotions()[index].getMana() > maxMana)
                {
                    // Set the character's mana to the max mana
                    mana = maxMana;
                }
                else
                {
                    // Add the mana to the character's mana
                    mana += inventory.getPotions()[index].getMana();
                }

                // Message
                std::cout << "Вы выпили " << inventory.getPotions()[index].getName() << "." << std::endl;
                std::cout << "Ваш запас маны теперь: " << mana << "." << std::endl;

                // Remove the potion from the inventory
                inventory.removePotion(index);
            }
        }
        // Check if the potion adds stamina
        if (inventory.getPotions()[index].getStamina() > 0)
        {
            // Check if the character is at full stamina
            if (stamina == maxStamina)
            {
                std::cout << "У вас уже максимальный запас выносливости." << std::endl;
            }
            else
            {
                // Check if the character's stamina will be over the max stamina
                if (stamina + inventory.getPotions()[index].getStamina() > maxStamina)
                {
                    // Set the character's stamina to the max stamina
                    stamina = maxStamina;
                }
                else
                {
                    // Add the stamina to the character's stamina
                    stamina += inventory.getPotions()[index].getStamina();
                }

                // Message
                std::cout << "Вы выпили " << inventory.getPotions()[index].getName() << "." << std::endl;
                std::cout << "Ваша выносливость теперь: " << stamina << "." << std::endl;

                // Remove the potion from the inventory
                inventory.removePotion(index);
            }
        }
    }
}

/**
 * @brief Снять оружие
 */

void Character::unequipWeapon()
{
    if (equippedWeapon.getName() == "None")
    {
        std::cout << "Оружие не экипировано." << std::endl;
    }
    else
    {
        damage -= equippedWeapon.getDamage();
        inventory.addWeapon(equippedWeapon);
        equippedWeapon = Weapon();
        std::cout << "Оружие снято и возвращено в инвентарь." << std::endl;
    }
}

/**
 * @brief Снять доспехи
 */

void Character::unequipArmor()
{
    if (equippedArmor.getName() == "None")
    {
        std::cout << "Доспехи не экипированы." << std::endl;
    }
    else
    {
        defense -= equippedArmor.getDefense();
        inventory.addArmor(equippedArmor);
        equippedArmor = Armor();
        std::cout << "Доспехи сняты и возвращены в инвентарь." << std::endl;
    }
}

int Character::getLevel() { return level; }
int Character::getExperience() { return experience; }
int Character::getNextLevelExperience() { return nextLevelExperience; }
void Character::setLevel(int lvl) { level = lvl; }
void Character::setExperience(int xp) { experience = xp; }
void Character::setNextLevelExperience(int nextXp) { nextLevelExperience = nextXp; }

bool Character::addExperience(int xp)
{
    experience += xp;
    bool leveledUp = false;
    while (experience >= nextLevelExperience)
    {
        experience -= nextLevelExperience;
        level++;
        nextLevelExperience = level * 150;
        int hpGain = 10 + (stats.getConstitution() / 2);
        maxHealth += hpGain;
        health = maxHealth;
        int manaGain = 10 + (stats.getIntelligence() / 2);
        maxMana += manaGain;
        mana = maxMana;
        stats.increaseStats(1, 1, 1, 1, 1, 1);
        damage = equippedWeapon.getDamage() + (stats.getStrength() / 2);
        defense = equippedArmor.getDefense() + (stats.getDexterity() / 2);
        leveledUp = true;
    }
    return leveledUp;
}

Race Character::getRace() { return race; }
void Character::setRace(Race r) { race = r; }
Class Character::getClass() { return clss; }
void Character::setClass(Class c) { clss = c; }
Stats Character::getStats() { return stats; }
void Character::setStats(Stats s) { stats = s; }
Weapon Character::getEquippedWeapon() { return equippedWeapon; }
void Character::setEquippedWeapon(Weapon w) { equippedWeapon = w; }
Armor Character::getEquippedArmor() { return equippedArmor; }
void Character::setEquippedArmor(Armor a) { equippedArmor = a; }
Inventory Character::getInventory() { return inventory; }
void Character::setInventory(Inventory inv) { inventory = inv; }

#endif // End of the preprocessor CHARACTER_CC