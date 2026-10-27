#ifndef MENU_CC
#define MENU_CC

#include "Ascii.cc"
#include "Initializer.cc"
#include "Functions.cc"

/**
 * @brief Отображение главного меню
 */
void mainMenu()
{
    cleanScreen();
    std::cout << readAsciiArt("asciiArt/title.txt") << std::endl;

    std::cout << "1. Создать персонажа" << std::endl;
    std::cout << "2. Показать лист персонажа" << std::endl;
    std::cout << "3. Экипированные предметы" << std::endl;
    std::cout << "4. Инвентарь" << std::endl;
    std::cout << "5. Торговец Боб" << std::endl;
    std::cout << "6. Энциклопедия D&D (Расы, Классы, Бестиарий)" << std::endl;
    std::cout << "7. ⚔️  Боевая Арена (Пошаговые дуэли)" << std::endl;
    std::cout << "8. 🏰  Поход в Подземелье (Dungeon Crawl)" << std::endl;
    std::cout << "9. 💾  Сохранить игру" << std::endl;
    std::cout << "10. 📂 Загрузить сохранение" << std::endl;
    std::cout << std::endl;

    std::cout << "0. Выход" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Меню энциклопедии рас
 */
void racesMenu()
{
    int option;
    std::vector<Race> races = initRaces();
    std::vector<std::string> racesNames = listRaces();
    int racesNumber = races.size();

    do
    {
        cleanScreen();
        std::cout << "=== Энциклопедия рас D&D ===" << std::endl;
        std::cout << std::endl;

        printStringVector(racesNames);
        std::cout << std::endl;

        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите расу для просмотра: ";
        std::cin >> option;

        if (option > 0 && option <= racesNumber)
        {
            cleanScreen();
            std::cout << races[option - 1].toString() << std::endl;
            pause();
        }
        else if (option == 0)
        {
            // Возврат
        }
        else
        {
            std::cout << "Неверный выбор." << std::endl;
            pause();
        }

    } while (option != 0);
}

/**
 * @brief Меню энциклопедии классов
 */
void classesMenu()
{
    int option;
    std::vector<Class> classes = initClasses();
    std::vector<std::string> classesNames = listClasses();
    int classesNumber = classes.size();

    do
    {
        cleanScreen();
        std::cout << "=== Энциклопедия классов D&D ===" << std::endl;
        std::cout << std::endl;

        printStringVector(classesNames);
        std::cout << std::endl;

        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите класс для просмотра: ";
        std::cin >> option;

        if (option > 0 && option <= classesNumber)
        {
            cleanScreen();
            std::cout << classes[option - 1].toString() << std::endl;
            pause();
        }
        else if (option == 0)
        {
            // Возврат
        }
        else
        {
            std::cout << "Неверный выбор." << std::endl;
            pause();
        }

    } while (option != 0);
}

/**
 * @brief Меню энциклопедии оружия
 */
void weaponsMenu()
{
    int option;
    std::vector<Weapon> weapons = initWeapons();
    std::vector<std::string> weaponsNames = listWeapons();
    int weaponsNumber = weapons.size();

    do
    {
        cleanScreen();
        std::cout << "=== Энциклопедия оружия D&D ===" << std::endl;
        std::cout << std::endl;

        printStringVector(weaponsNames);
        std::cout << std::endl;

        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите оружие для просмотра: ";
        std::cin >> option;

        if (option > 0 && option <= weaponsNumber)
        {
            cleanScreen();
            std::cout << weapons[option - 1].toString() << std::endl;
            pause();
        }
        else if (option == 0)
        {
            // Возврат
        }
        else
        {
            std::cout << "Неверный выбор." << std::endl;
            pause();
        }

    } while (option != 0);
}

/**
 * @brief Меню энциклопедии доспехов
 */
void armorMenu()
{
    int option;
    std::vector<Armor> armors = initArmors();
    std::vector<std::string> armorsNames = listArmors();
    int armorsNumber = armors.size();

    do
    {
        cleanScreen();
        std::cout << "=== Энциклопедия доспехов D&D ===" << std::endl;
        std::cout << std::endl;

        printStringVector(armorsNames);
        std::cout << std::endl;

        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите доспех для просмотра: ";
        std::cin >> option;

        if (option > 0 && option <= armorsNumber)
        {
            cleanScreen();
            std::cout << armors[option - 1].toString() << std::endl;
            pause();
        }
        else if (option == 0)
        {
            // Возврат
        }
        else
        {
            std::cout << "Неверный выбор." << std::endl;
            pause();
        }

    } while (option != 0);
}

/**
 * @brief Меню энциклопедии зелий
 */
void potionsMenu()
{
    int option;
    std::vector<Potion> potions = initPotions();
    std::vector<std::string> potionsNames = listPotions();
    int potionsNumber = potions.size();

    do
    {
        cleanScreen();
        std::cout << "=== Энциклопедия зелий D&D ===" << std::endl;
        std::cout << std::endl;

        printStringVector(potionsNames);
        std::cout << std::endl;

        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите зелье для просмотра: ";
        std::cin >> option;

        if (option > 0 && option <= potionsNumber)
        {
            cleanScreen();
            std::cout << potions[option - 1].toString() << std::endl;
            pause();
        }
        else if (option == 0)
        {
            // Возврат
        }
        else
        {
            std::cout << "Неверный выбор." << std::endl;
            pause();
        }

    } while (option != 0);
}

/**
 * @brief Меню бестиария (монстров)
 */
void bestiaryMenu()
{
    int option;
    std::vector<Monster> monsters = initMonsters();
    int monstersNumber = monsters.size();

    do
    {
        cleanScreen();
        std::cout << "=== Энциклопедия монстров (Бестиарий D&D) ===" << std::endl;
        std::cout << std::endl;

        for (int i = 0; i < monstersNumber; ++i)
        {
            std::cout << (i + 1) << ". " << monsters[i].getName() 
                      << " [" << monsters[i].getType() << "]" << std::endl;
        }
        std::cout << std::endl;
        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите монстра для изучения: ";
        std::cin >> option;

        if (option > 0 && option <= monstersNumber)
        {
            cleanScreen();
            std::cout << monsters[option - 1].toString() << std::endl;
            std::cout << monsters[option - 1].getAsciiArt() << std::endl;
            pause();
        }
        else if (option != 0)
        {
            std::cout << "Неверный выбор." << std::endl;
            pause();
        }

    } while (option != 0);
}

/**
 * @brief Главное меню энциклопедии
 */
void encyclopediaMenu()
{
    int option;

    do
    {
        cleanScreen();
        std::cout << readAsciiArt("asciiArt/encyclopedia.txt") << std::endl;

        std::cout << "1. Расы" << std::endl;
        std::cout << "2. Классы" << std::endl;
        std::cout << "3. Оружие" << std::endl;
        std::cout << "4. Доспехи" << std::endl;
        std::cout << "5. Зелья" << std::endl;
        std::cout << "6. Бестиарий (Монстры)" << std::endl;
        std::cout << std::endl;

        std::cout << "0. Назад" << std::endl;
        std::cout << std::endl;

        std::cout << "Выберите раздел: ";
        std::cin >> option;

        switch (option)
        {
        case 1:
            racesMenu();
            break;
        case 2:
            classesMenu();
            break;
        case 3:
            weaponsMenu();
            break;
        case 4:
            armorMenu();
            break;
        case 5:
            potionsMenu();
            break;
        case 6:
            bestiaryMenu();
            break;
        case 0:
            std::cout << "Возвращайтесь, если захотите освежить знания по миру D&D!" << std::endl;
            break;
        default:
            std::cout << "Неверный выбор раздела." << std::endl;
            pause();
            break;
        }

    } while (option != 0);
}

#endif // MENU_CC