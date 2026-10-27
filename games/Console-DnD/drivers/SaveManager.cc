#ifndef SAVEMANAGER_CC
#define SAVEMANAGER_CC

#include "../headers/SaveManager.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "Initializer.cc"

bool SaveManager::hasSave(const std::string& filename)
{
    std::ifstream file(filename);
    return file.good();
}

bool SaveManager::saveCharacter(Character& character, const std::string& filename)
{
    using json = nlohmann::json;

    json j;
    j["name"] = character.getName();
    j["gender"] = character.getGender();
    j["age"] = character.getAge();
    j["alignment"] = character.getAlignment();

    j["level"] = character.getLevel();
    j["experience"] = character.getExperience();
    j["nextLevelExperience"] = character.getNextLevelExperience();

    j["health"] = character.getHealth();
    j["maxHealth"] = character.getMaxHealth();
    j["mana"] = character.getMana();
    j["maxMana"] = character.getMaxMana();
    j["stamina"] = character.getStamina();
    j["maxStamina"] = character.getMaxStamina();

    j["damage"] = character.getDamage();
    j["defense"] = character.getDefense();
    j["gold"] = character.getGold();

    // Race & Class
    j["race"] = character.getRace().getName();
    j["class"] = character.getClass().getName();

    // Stats
    Stats st = character.getStats();
    j["stats"] = {
        {"strength", st.getStrength()},
        {"dexterity", st.getDexterity()},
        {"constitution", st.getConstitution()},
        {"intelligence", st.getIntelligence()},
        {"wisdom", st.getWisdom()},
        {"charisma", st.getCharisma()}
    };

    // Equipped weapon
    Weapon ew = character.getEquippedWeapon();
    j["equippedWeapon"] = {
        {"name", ew.getName()},
        {"description", ew.getDescription()},
        {"rarity", ew.getRarity()},
        {"weight", ew.getWeight()},
        {"price", ew.getPrice()},
        {"subtype", ew.getSubtype()},
        {"damage", ew.getDamage()}
    };

    // Equipped armor
    Armor ea = character.getEquippedArmor();
    j["equippedArmor"] = {
        {"name", ea.getName()},
        {"description", ea.getDescription()},
        {"rarity", ea.getRarity()},
        {"weight", ea.getWeight()},
        {"price", ea.getPrice()},
        {"subtype", ea.getSubtype()},
        {"defense", ea.getDefense()}
    };

    // Inventory items
    Inventory inv = character.getInventory();
    json weaponsJson = json::array();
    for (auto& w : inv.getWeapons())
    {
        weaponsJson.push_back({
            {"name", w.getName()},
            {"description", w.getDescription()},
            {"rarity", w.getRarity()},
            {"weight", w.getWeight()},
            {"price", w.getPrice()},
            {"subtype", w.getSubtype()},
            {"damage", w.getDamage()}
        });
    }
    j["inventory_weapons"] = weaponsJson;

    json armorsJson = json::array();
    for (auto& a : inv.getArmors())
    {
        armorsJson.push_back({
            {"name", a.getName()},
            {"description", a.getDescription()},
            {"rarity", a.getRarity()},
            {"weight", a.getWeight()},
            {"price", a.getPrice()},
            {"subtype", a.getSubtype()},
            {"defense", a.getDefense()}
        });
    }
    j["inventory_armors"] = armorsJson;

    json potionsJson = json::array();
    for (auto& p : inv.getPotions())
    {
        potionsJson.push_back({
            {"name", p.getName()},
            {"description", p.getDescription()},
            {"rarity", p.getRarity()},
            {"weight", p.getWeight()},
            {"price", p.getPrice()},
            {"health", p.getHealth()},
            {"mana", p.getMana()},
            {"stamina", p.getStamina()}
        });
    }
    j["inventory_potions"] = potionsJson;

    // Write to file
    std::ofstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    file << j.dump(4);
    file.close();
    return true;
}

bool SaveManager::loadCharacter(Character& character, const std::string& filename)
{
    using json = nlohmann::json;

    std::ifstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    try
    {
        json j = json::parse(file);
        file.close();

        character.setName(j.value("name", "Герой"));
        character.setGender(j.value("gender", "Мужской"));
        character.setAge(j.value("age", 25));
        character.setAlignment(j.value("alignment", "Истинно нейтральный"));

        character.setLevel(j.value("level", 1));
        character.setExperience(j.value("experience", 0));
        character.setNextLevelExperience(j.value("nextLevelExperience", 100));

        character.setMaxHealth(j.value("maxHealth", 100));
        character.setHealth(j.value("health", 100));
        character.setMaxMana(j.value("maxMana", 100));
        character.setMana(j.value("mana", 100));
        character.setMaxStamina(j.value("maxStamina", 100));
        character.setStamina(j.value("stamina", 100));

        character.setDamage(j.value("damage", 10));
        character.setDefense(j.value("defense", 10));

        // Restore Race
        std::string raceName = j.value("race", "Человек");
        std::vector<Race> allRaces = initRaces();
        for (auto& r : allRaces)
        {
            if (r.getName() == raceName)
            {
                character.setRace(r);
                break;
            }
        }

        // Restore Class
        std::string className = j.value("class", "Воин");
        std::vector<Class> allClasses = initClasses();
        for (auto& c : allClasses)
        {
            if (c.getName() == className)
            {
                character.setClass(c);
                break;
            }
        }

        // Restore Stats
        if (j.contains("stats"))
        {
            Stats st = character.getStats();
            st.setStrength(j["stats"].value("strength", 10));
            st.setDexterity(j["stats"].value("dexterity", 10));
            st.setConstitution(j["stats"].value("constitution", 10));
            st.setIntelligence(j["stats"].value("intelligence", 10));
            st.setWisdom(j["stats"].value("wisdom", 10));
            st.setCharisma(j["stats"].value("charisma", 10));
            character.setStats(st);
        }

        // Restore Equipped Weapon
        if (j.contains("equippedWeapon"))
        {
            auto& ewj = j["equippedWeapon"];
            Weapon ew(
                ewj.value("name", "None"),
                ewj.value("description", ""),
                ewj.value("rarity", "Common"),
                ewj.value("weight", 1.0f),
                ewj.value("price", 10),
                ewj.value("subtype", "Simple"),
                ewj.value("damage", 5)
            );
            character.setEquippedWeapon(ew);
        }

        // Restore Equipped Armor
        if (j.contains("equippedArmor"))
        {
            auto& eaj = j["equippedArmor"];
            Armor ea(
                eaj.value("name", "None"),
                eaj.value("description", ""),
                eaj.value("rarity", "Common"),
                eaj.value("weight", 2.0f),
                eaj.value("price", 10),
                eaj.value("subtype", "Light"),
                eaj.value("defense", 5)
            );
            character.setEquippedArmor(ea);
        }

        // Restore Inventory
        Inventory inv;
        inv.setGold(j.value("gold", 50));

        if (j.contains("inventory_weapons"))
        {
            for (auto& wj : j["inventory_weapons"])
            {
                Weapon w(
                    wj.value("name", ""),
                    wj.value("description", ""),
                    wj.value("rarity", "Common"),
                    wj.value("weight", 1.0f),
                    wj.value("price", 10),
                    wj.value("subtype", "Simple"),
                    wj.value("damage", 5)
                );
                inv.addWeapon(w);
            }
        }

        if (j.contains("inventory_armors"))
        {
            for (auto& aj : j["inventory_armors"])
            {
                Armor a(
                    aj.value("name", ""),
                    aj.value("description", ""),
                    aj.value("rarity", "Common"),
                    aj.value("weight", 2.0f),
                    aj.value("price", 10),
                    aj.value("subtype", "Light"),
                    aj.value("defense", 5)
                );
                inv.addArmor(a);
            }
        }

        if (j.contains("inventory_potions"))
        {
            for (auto& pj : j["inventory_potions"])
            {
                Potion p(
                    pj.value("name", ""),
                    pj.value("description", ""),
                    pj.value("rarity", "Common"),
                    pj.value("weight", 0.5f),
                    pj.value("price", 10),
                    pj.value("health", 25),
                    pj.value("mana", 0),
                    pj.value("stamina", 0)
                );
                inv.addPotion(p);
            }
        }

        character.setInventory(inv);
        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << "Ошибка при загрузке сохранения: " << e.what() << std::endl;
        return false;
    }
}

std::string SaveManager::getSaveInfo(const std::string& filename)
{
    using json = nlohmann::json;
    std::ifstream file(filename);
    if (!file.is_open()) return "Сохранение не найдено.";

    try
    {
        json j = json::parse(file);
        std::string name = j.value("name", "Герой");
        int lvl = j.value("level", 1);
        std::string cl = j.value("class", "Воин");
        std::string rc = j.value("race", "Человек");
        int gold = j.value("gold", 0);
        return name + " (" + rc + " / " + cl + "), Уровень " + std::to_string(lvl) + ", Золото: " + std::to_string(gold);
    }
    catch (...)
    {
        return "Повреждённый файл сохранения.";
    }
}

bool SaveManager::deleteSave(const std::string& filename)
{
    return std::remove(filename.c_str()) == 0;
}

#endif // SAVEMANAGER_CC
