#ifndef MAIN_H
#define MAIN_H

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>

#include "drivers/Menu.cc"
#include "drivers/Functions.cc"
#include "drivers/Character.cc"
#include "drivers/Merchant.cc"
#include "drivers/SaveManager.cc"
#include "drivers/CombatEngine.cc"

int main()
{
#ifdef _WIN32
    // Настройка кодировки UTF-8 для корректного вывода русского языка без искажений
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Seed random generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int mainOption = -1;
    Character character;
    Merchant merchant;

    // Инициализация торговца и товаров
    merchant.initialize();

    // Проверка наличия сохраненной игры при запуске
    if (SaveManager::hasSave())
    {
        cleanScreen();
        std::cout << "========================================================" << std::endl;
        std::cout << "💾 НАЙДЕНО СОХРАНЕНИЕ: " << SaveManager::getSaveInfo() << std::endl;
        std::cout << "========================================================" << std::endl;
        std::cout << "Хотите загрузить сохранённую игру?" << std::endl;
        std::cout << "1. Да, продолжить приключение" << std::endl;
        std::cout << "2. Нет, начать с нуля" << std::endl;
        std::cout << "Ваш выбор: ";
        int loadChoice = 0;
        if (std::cin >> loadChoice && loadChoice == 1)
        {
            if (SaveManager::loadCharacter(character))
            {
                std::cout << "Герой " << character.getName() << " успешно загружен в игру!" << std::endl;
            }
        }
        std::cin.clear();
        pause();
    }

    // Главный цикл меню
    do
    {
        mainMenu();

        std::cout << "Выберите действие: ";
        if (!(std::cin >> mainOption))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            mainOption = -1;
            cleanScreen();
            std::cout << "Пожалуйста, введите числовое значение." << std::endl;
            pause();
            continue;
        }
        cleanScreen();

        switch (mainOption)
        {
        case 1:
            if (character.getName() != "None")
            {
                int characterOption = 0;
                std::cout << "Персонаж уже создан!" << std::endl;
                std::cout << "Хотите создать нового и перезаписать текущего?" << std::endl;
                std::cout << "1. Да" << std::endl;
                std::cout << "2. Нет" << std::endl;
                std::cout << "Ваш выбор: ";
                std::cin >> characterOption;

                switch (characterOption)
                {
                case 1:
                    character.createCharacter();
                    break;
                case 2:
                    std::cout << "Создание отменено. Персонаж сохранён." << std::endl;
                    break;
                default:
                    std::cout << "Неверный вариант." << std::endl;
                    break;
                }
            }
            else
            {
                character.createCharacter();
            }
            pause();
            break;
        case 2:
            if (character.getName() == "None")
            {
                std::cout << "Персонаж ещё не создан." << std::endl;
            }
            else
            {
                std::cout << character.toString() << std::endl;
            }
            pause();
            break;
        case 3:
            if (character.getName() == "None")
            {
                std::cout << "Персонаж ещё не создан." << std::endl;
            }
            else
            {
                character.showEquippedItems();
            }
            pause();
            break;
        case 4:
            if (character.getName() == "None")
            {
                std::cout << "Персонаж ещё не создан." << std::endl;
            }
            else
            {
                character.showInventory();
            }
            pause();
            break;
        case 5:
            if (character.getName() == "None")
            {
                std::cout << "Сначала необходимо создать персонажа!" << std::endl;
            }
            else
            {
                merchant.interact(character);
            }
            pause();
            break;
        case 6:
            encyclopediaMenu();
            break;
        case 7:
            CombatEngine::arenaMenu(character);
            break;
        case 8:
            CombatEngine::dungeonCrawl(character);
            break;
        case 9:
            if (character.getName() == "None")
            {
                std::cout << "Сначала необходимо создать персонажа!" << std::endl;
            }
            else
            {
                if (SaveManager::saveCharacter(character))
                {
                    std::cout << "💾 Игра успешно сохранена!" << std::endl;
                    std::cout << "Персонаж: " << character.getName()
                              << " [Уровень " << character.getLevel() << ", Золото: "
                              << character.getGold() << "]" << std::endl;
                }
                else
                {
                    std::cout << "Ошибка при записи файла сохранения!" << std::endl;
                }
            }
            pause();
            break;
        case 10:
            if (SaveManager::hasSave())
            {
                if (SaveManager::loadCharacter(character))
                {
                    std::cout << "📂 Сохранение успешно загружено!" << std::endl;
                    std::cout << "Персонаж: " << character.getName()
                              << " [Уровень " << character.getLevel() << ", "
                              << character.getClass().getName() << "]" << std::endl;
                }
                else
                {
                    std::cout << "Ошибка при чтении файла сохранения!" << std::endl;
                }
            }
            else
            {
                std::cout << "Файл сохранения (data/savegame.json) не найден." << std::endl;
            }
            pause();
            break;
        case 0:
            std::cout << "До свидания, искатель приключений! До скорых встреч..." << std::endl;
            pause();
            break;
        default:
            std::cout << "Неверный пункт меню. Попробуйте снова." << std::endl;
            pause();
            break;
        }
        cleanScreen();
    } while (mainOption != 0);

    return 0;
}

#endif // MAIN_H