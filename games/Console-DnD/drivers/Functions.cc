#ifndef FUNCTIONS_CC
#define FUNCTIONS_CC

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>

/**
 * @brief Очистка экрана консоли (кроссплатформенная)
 */
void cleanScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Пауза в консоли с корректным выводом на русском языке
 */
void pause()
{
    std::cout << "\nНажмите любую клавишу для продолжения..." << std::endl;
#ifdef _WIN32
    system("pause > nul");
#else
    std::cin.clear();
    std::cin.get();
#endif
}

/**
 * @brief Вывод элементов вектора строк с нумерацией
 *
 * @param v Вектор строк
 */
void printStringVector(std::vector<std::string> v)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        std::cout << i + 1 << ". " << v[i] << std::endl;
    }
}

#endif // FUNCTIONS_CC