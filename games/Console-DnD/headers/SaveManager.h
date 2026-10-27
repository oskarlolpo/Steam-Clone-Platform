#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <string>
#include "../headers/Character.h"

class SaveManager
{
public:
    // Check if savegame file exists
    static bool hasSave(const std::string& filename = "data/savegame.json");

    // Save character to JSON
    static bool saveCharacter(Character& character, const std::string& filename = "data/savegame.json");

    // Load character from JSON
    static bool loadCharacter(Character& character, const std::string& filename = "data/savegame.json");

    // Get basic summary of saved character (name, level, class) without loading
    static std::string getSaveInfo(const std::string& filename = "data/savegame.json");

    // Delete savegame
    static bool deleteSave(const std::string& filename = "data/savegame.json");
};

#endif // SAVEMANAGER_H
