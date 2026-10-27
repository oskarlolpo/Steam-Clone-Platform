# Steam Clone Platform & Games Ecosystem

Комплексный проект, включающий собственный клиент **Steam Clone** на C++20 и подключенные к нему игры, включая 3D FPS шутер (**CS2 Demake**) на C# Windows Forms и консольную RPG (**Console-DnD**).

---

## 📁 Структура Репозитория

```text
Steam-Clone-Platform/
├── client/                     # Клиент Steam Clone (C++20, Win32 / CMake)
│   ├── CMakeLists.txt          # Скрипт сборки CMake
│   ├── build.bat / run.bat     # Скрипты сборки и запуска
│   ├── src/
│   │   ├── core/               # Модели манифестов, типы, SHA-256 хеширование
│   │   ├── downloader/         # Движок многопоточной загрузки (Chunked / Range)
│   │   ├── launcher/           # Win32 Process Launcher & трекинг времени игры
│   │   ├── storage/            # Хранилище манифестов и библиотека (JSON)
│   │   └── ui/                 # Win32 GUI интерфейс клиента Steam
│   └── steam_data/             # Файл библиотеки игр (library.json)
│
├── games/
│   ├── CS2WinFormsDemo/        # 3D FPS Шутер (C# / .NET 9 WinForms)
│   │   ├── CS2WinFormsDemo.csproj
│   │   ├── RaycastRenderer.cs  # Кастомный DDA-движок (прыжки, присядка, обзор)
│   │   ├── Weapon.cs           # 3D оружие (AK-47, Deagle, Нож) с отдачей
│   │   ├── Player.cs           # Физика игрока, перемещение, коллизии
│   │   ├── Enemy.cs            # Боты, регистрация попаданий, хедшоты
│   │   ├── Door.cs             # Интерактивные раздвижные двери
│   │   ├── GameMap.cs          # Карта арены в стиле Dust 2
│   │   ├── TextureManager.cs   # Процедурные текстуры
│   │   └── SoundManager.cs     # Процедурные звуковые эффекты (WAV)
│   │
│   └── Console-DnD/            # Dungeons & Dragons Console RPG (C++)
│       ├── CMakeLists.txt
│       ├── main.cpp
│       └── headers/ / data/
│
└── README.md
```

---

## 🚀 Быстрый старт

### 1. Запуск и сборка клиента Steam Clone
Требования: Visual Studio 2022 (MSVC C++20), CMake.
```cmd
cd client
build.bat
run.bat
```

### 2. Запуск 3D FPS Шутера (CS2 Demake)
Требования: .NET 9 SDK.
```cmd
cd games/CS2WinFormsDemo
dotnet run
```

---

## 🎮 Возможности

- **Клиент Steam (C++20):**
  - Локальная библиотека и хранение данных (`library.json`).
  - Win32 GUI с кастомным рендерингом списка игр и баннеров.
  - Запуск, отслеживание PID и учёт игрового времени процессов.
  - Прямая интеграция и запуск установленных игр.

- **3D FPS Шутер (C# WinForms):**
  - Быстрый софтверный DDA Raycaster с прямой записью в видеопамять (`Bitmap.LockBits`).
  - Полный обзор мышью (включая наклон камеры вверх и вниз — Pitch).
  - Прыжки (Space) и тактическая присядка (Ctrl / C).
  - Интерактивные раздвижные двери (клавиша `E` или авто-открытие).
  - Модели оружия в аутентичной перспективе от первого лица.
  - Боты с регистрацией хедшотов, хитмаркером и Killfeed.
