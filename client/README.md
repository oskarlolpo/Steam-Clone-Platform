# Steam Clone Client Engine (C++20 Architecture)

Минимально жизнеспособный дистрибутив цифрового контента (Steam Clone Core) на C++20.

---

## 🏗 Архитектура подсистем

```
steam/
├── CMakeLists.txt                      # Корневой CMake (C++20, warnings, targets)
├── src/
│   ├── core/                           # Доменные модели, типы, криптография SHA-256
│   │   ├── include/steam/core/
│   │   │   ├── Types.hpp               # AppId, UserId, AppState
│   │   │   ├── AppManifest.hpp         # Метаданные игры, версия, пути, хеши
│   │   │   ├── UserSession.hpp         # Авторизация, сессионные токены
│   │   │   └── Sha256.hpp              # Независимый криптографический SHA-256
│   │   └── src/
│   │       └── Sha256.cpp
│   │
│   ├── downloader/                     # Модуль загрузки (Download Engine)
│   │   ├── include/steam/downloader/
│   │   │   ├── DownloadTypes.hpp       # Progress, Request, Status
│   │   │   ├── IDownloadManager.hpp    # Абстрактный интерфейс
│   │   │   └── DownloadManager.hpp     # Chunked/Range загрузчик
│   │   └── src/
│   │       └── DownloadManager.cpp     # Многопоточный пул (std::jthread), Range, SHA-256
│   │
│   ├── launcher/                       # Диспетчер запуска и мониторинга процессов
│   │   ├── include/steam/launcher/
│   │   │   ├── ProcessTypes.hpp        # ProcessState, LaunchConfig, ProcessExecutionInfo
│   │   │   ├── IProcessLauncher.hpp    # Абстрактный интерфейс
│   │   │   └── ProcessLauncher.hpp     # Windows CreateProcessW / POSIX fork-exec
│   │   └── src/
│   │       └── ProcessLauncher.cpp     # Асинхронный мониторинг PID и подсчет времени
│   │
│   ├── storage/                        # Локальное хранилище и кэш состояния
│   │   ├── include/steam/storage/
│   │   │   ├── ILocalStorage.hpp       # Абстрактный интерфейс хранилища
│   │   │   └── JsonStorage.hpp         # Атомарная запись (safe atomic rename)
│   │   └── src/
│   │       └── JsonStorage.cpp
│   │
│   └── main.cpp                        # Сквозное демо всех подсистем
```

---

## ⚡ Особенности реализации

1. **C++20 / C++23 стандарт**:
   - Использование `std::jthread` со `std::stop_token` для автоматического RAII-управления потоками воркеров.
   - Использование `std::span`, `std::bit`, `std::filesystem`, `<chrono>`.
2. **Память и безопасность**:
   - Строгий **SOLID** и **RAII**.
   - Полное отсутствие сырых указателей: `std::unique_ptr` и `std::shared_ptr`.
   - Потокобезопасная синхронизация через `std::mutex`, `std::scoped_lock`, `std::condition_variable_any`.
3. **Download Engine**:
   - Чанковая загрузка с разбивкой файла на блоки (Range queries).
   - Поддержка паузы (`pause`), возобновления (`resume`), отмены (`cancel`).
   - Ограничение скорости (Bandwidth Throttling).
   - Автоматическая проверка целостности хеша SHA-256 после завершения загрузки.
4. **Process Launcher**:
   - Запуск через `CreateProcessW` с поддержкой аргументов и рабочих директорий.
   - Отслеживание жизненного цикла процесса (PID, ExitCode, Crashed/Exited).
   - Подсчет игрового времени сессии с точностью до секунды и автоматическая синхронизация с локальной БД.
5. **Storage & State**:
   - Атомарное сохранение в JSON (запись во временный файл + атомарный `rename`).

---

## 🛠 Сборка проекта

### Требования
- Компилятор с поддержкой C++20: MSVC (Visual Studio 2022+), GCC 11+ или Clang 13+
- CMake 3.22+

### Сборка через консоль (Visual Studio Developer Command Prompt / MSYS2 / WSL):
```bash
cd c:/Users/govno/Desktop/zadaniy/steam
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Запуск клиента

- **Графический интерфейс (GUI)**:
  Запустите `run.bat` или дважды кликните по `build/bin/Release/steam_client.exe`.
  Откроется окно Steam Desktop с темной темой, списком игр, кнопками «Установить» / «Играть», живым прогресс-баром и счетчиком игрового времени.

- **Консольный режим (CLI Demo)**:
  ```bash
  .\build\bin\Release\steam_client.exe --cli
  ```
