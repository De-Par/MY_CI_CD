# awesome_calc

Учебно-боевой пример C++-проекта с **полноценным CI/CD на GitHub**.

Проект специально сделан простым по коду, но «по-взрослому» по инфраструктуре:

* C++17, **Meson + Ninja**
* библиотека `awesome_calc` + CLI-утилита
* тесты на **GoogleTest**
* **CI** на GitHub Actions:

  * Linux / macOS / Windows
  * clang-format (стиль кода)
  * clang-tidy (статический анализ)
  * сборка + юнит-тесты (debug/release)
  * coverage (gcovr)
  * санитайзеры (ASan/UBSan)
* **CD**:

  * релизы по git-тегам с готовыми архивами

README одновременно:

1. Документация к проекту.
2. Мини-курс по CI/CD и GitHub Actions «на примере».

---

## 0. Краткое резюме CI/CD

* Триггеры: push/PR в `main`, nightly санитайзеры, ручной `workflow_dispatch`, релизы по тегам `v*`.
* Охват ОС: Linux, macOS, Windows (матрица в CI и релизах).
* Проверки: clang-format, clang-tidy (warnings-as-errors), Meson-сборка и юнит-тесты в `debug`/`release`.
* Дополнительно: coverage (gcovr, артефакты HTML/XML), ASan/UBSan (ручной или по расписанию).
* Релизы: `meson install` → артефакты (`tar.gz` для *nix, `zip` для Windows) автоматически прикрепляются к GitHub Release.
* Автоматическое окружение: composite action ставит Python+Meson+Ninja и подтягивает wrap для GoogleTest, чтобы сборка не зависела от конкретной ОС.

---

## 1. Что делает сам проект

### 1.1. Библиотека `awesome_calc`

Простая библиотека с двумя функциями:

```cpp
namespace awesome_calc {

int add(int a, int b);                     // Сложение двух целых чисел

double mean(const std::vector<double>&);   // Среднее арифметическое вектора

} // namespace awesome_calc
```

Особенности:

* `mean` выбрасывает `std::invalid_argument`, если вектор пустой (это покрыто тестами).
* Код оформлен и форматирован по `.clang-format`.

### 1.2. CLI-утилита `awesome_calc_cli`

Консольная программа, которая:

* вызывает `add` и `mean`,
* печатает результаты в stdout.

Это просто пример **реального использования библиотеки** — такой небольшой end-to-end сценарий.

### 1.3. Юнит-тесты (GoogleTest)

Тесты проверяют:

* корректность `add` на разных значениях (включая отрицательные),
* корректность `mean` для непустого вектора,
* выброс исключения `std::invalid_argument` при пустом векторе.

---

## 2. Что такое CI/CD (теория на пальцах)

### 2.1. CI — Continuous Integration

**CI (Continuous Integration)** — это практика, когда:

* каждый push / pull request в репозиторий автоматически:

  1. проверяется линтерами / форматтерами;
  2. собирается;
  3. прогоняет тесты;
  4. иногда ещё делает статический анализ и т.п.

Цель: **узнать о проблеме как можно раньше**, а не «в конце спринта».

В этом проекте CI делает:

* формат-чек (clang-format),
* статический анализ (clang-tidy),
* сборку и тесты на Linux/macOS/Windows в debug и release.

### 2.2. CD — Continuous Delivery / Deployment

**CD (Continuous Delivery / Deployment)** — это:

* автоматизация этапа «сборка релиза»:

  * упаковка артефактов,
  * создание релизов,
  * при желании деплой на серверы / магазины / репозитории пакетов.

Здесь CD реализовано как:

* автоматический **GitHub Release при пуше git-тега `v*`**,
* сборка бинарей, `meson install`, упаковка в архивы и прикрепление их к релизу.

---

## 3. Используемые инструменты

### 3.1. Meson + Ninja

* **Meson** — современная система сборки (альтернатива CMake).
* **Ninja** — быстрый низкоуровневый сборщик (генерируется Meson).

Основные команды:

```bash
# Конфигурируем проект
meson setup build

# Собираем
meson compile -C build

# Запускаем тесты
meson test -C build --print-errorlogs
```

### 3.2. GoogleTest

Фреймворк для юнит-тестов на C++. В проекте подключается через Meson (обычно через `meson wrap install gtest`).

### 3.3. clang-format

Инструмент автоформатирования кода C/C++:

* Выравнивание отступов
* Стиль скобок
* Расположение `#include` и т.п.

В CI используется в режиме **проверки** (ничего не меняет, просто падает, если формат неправильный).

### 3.4. clang-tidy

Статический анализатор:

* ловит потенциальные ошибки,
* показывает плохие практики,
* может подсказать оптимизации.

В CI настроен так, что **любое предупреждение считается ошибкой**.

### 3.5. Санитайзеры (ASan/UBSan)

Встроенные в компилятор режимы:

* **ASan (AddressSanitizer)**:

  * переполнения буфера,
  * use-after-free,
  * двойной delete и т.п.
* **UBSan (UndefinedBehaviorSanitizer)**:

  * undefined behaviour (деление на 0, выход за диапазон, и прочее).

Используются в отдельном workflow, чтобы не замедлять каждый PR.

### 3.6. gcovr

Инструмент для расчёта **покрытия кода**:

* смотрит, какие строки/ветки были выполнены тестами,
* может сгенерировать HTML-отчёт.

---

## 4. Структура репозитория

```text
.
├── .clang-format
├── .editorconfig
├── .gitignore
├── README.md
├── meson.build
├── meson.options
├── include/
│   └── awesome_calc/
│       └── calc.hpp       # Заголовок библиотеки
├── src/
│   ├── meson.build
│   └── calc.cpp           # Реализация библиотеки
├── app/
│   ├── meson.build
│   └── main.cpp           # CLI-приложение
├── tests/
│   ├── meson.build
│   └── test_calc.cpp      # Юнит-тесты (GoogleTest)
└── .github/
    ├── actions/
    │   └── setup-meson-env/
    │       └── action.yml  # composite action для Meson-окружения
    └── workflows/
        ├── ci.yml                      # основной CI
        ├── build-and-test-reusable.yml # reusable workflow
        ├── coverage.yml                # coverage-отчёты
        ├── sanitizers.yml              # ASan/UBSan
        └── release.yml                 # релизы по тегам
```

---

## 5. Как собрать и запустить локально

### 5.1. Установка зависимостей (Linux/macOS)

1. Установить компилятор (g++/clang).

2. Установить Python:

   ```bash
   python3 --version
   # если нет — установить через пакетный менеджер
   ```

3. Установить Meson и Ninja:

   ```bash
   python3 -m pip install --upgrade pip
   python3 -m pip install meson ninja
   ```

4. (Опционально) установить clang-format и clang-tidy:

   ```bash
   # Debian/Ubuntu
   sudo apt-get update
   sudo apt-get install -y clang-format clang-tidy
   ```

5. Установить GoogleTest (обычно через Meson WrapDB):

   ```bash
   meson wrap install gtest
   ```

   Это создаст файл `subprojects/gtest.wrap`, который Meson использует для скачивания и сборки GTest. В CI этот шаг делает composite action автоматически.

### 5.1.1. Windows (MSVC)

* Установите **Visual Studio Build Tools** или полную Visual Studio с C++ workload.
* Установите Python и Meson/Ninja через `pip` (как выше).
* В терминале x64 Native Tools (или под `Developer PowerShell`) выполните:

  ```powershell
  meson wrap install gtest
  meson setup build
  meson compile -C build
  meson test -C build --print-errorlogs
  ```

  В GitHub Actions активация MSVC делается шагом `msvc-dev-cmd`, локально его заменяет запуск из VS-инструментального терминала.

### 5.2. Сборка и запуск

```bash
# Конфигурируем (Debug по умолчанию)
meson setup build

# Сборка
meson compile -C build

# Запуск CLI
./build/awesome_calc_cli   # Linux/macOS
# или
.\build\awesome_calc_cli.exe  # Windows
```

### 5.3. Запуск тестов

```bash
meson test -C build --print-errorlogs
```

Если тесты падают, Meson покажет лог с информацией.

### 5.4. Release-сборка

```bash
meson setup build-release --buildtype=release
meson compile -C build-release
```

### 5.5. Форматирование перед push (опционально, «как делают в проде»)

**Рекомендуемо (через Meson run_target):**

1. Один раз сконфигурируйте: `meson setup build` (если не сделали раньше).
2. Добавьте git-hook, который вызывает Meson-цель:

   ```bash
   cat > .git/hooks/pre-push <<'HOOK'
   #!/usr/bin/env bash
   set -euo pipefail
   meson compile -C build format
   HOOK
   chmod +x .git/hooks/pre-push
   ```

   Цель `format` определяется в `meson.build` и использует clang-format. Такой подход типичен в больших компаниях: форматирование привязано к системе сборки, а не к отдельным скриптам.

**Альтернатива:** `meson format` / `meson format-check` (если clang-format установлен) или ручной вызов `./tools/clang-format-all.sh`.

---

## 6. GitHub Actions: как устроен CI/CD на практике

### 6.1. Базовые понятия

**GitHub Actions** — это встроенная система CI/CD в GitHub.

Основные термины:

* **Workflow** — сценарий в YAML-файле в `.github/workflows/*.yml`.

  * Примеры: `ci.yml`, `coverage.yml`, `release.yml`.
* **Job** — набор шагов, выполняемый на отдельной виртуальной машине («runner»).
* **Step** — конкретное действие: checkout кода, установка зависимостей, запуск команд.
* **Runner** — виртуальная машина, где всё это крутится:

  * `ubuntu-latest`, `macos-latest`, `windows-latest`.

**Триггеры** (поле `on:` в YAML):

* `push` — запуск при push в указанные ветки.
* `pull_request` — запуск при создании/обновлении PR.
* `workflow_dispatch` — ручной запуск из вкладки Actions.
* `schedule` — запуск по расписанию (cron).
* `push` с `tags` — запуск при пуше тегов (используем для релизов).

---

## 7. Обзор workflow’ов в проекте

### 7.1. `ci.yml` — основной CI

**Цель:** проверять каждый push/PR в ветку `main`.

Что делает:

1. **Lint (clang-format)**:

   * запускается на `ubuntu-latest`,
   * проверяет формат всех `*.cpp` и `*.hpp` в `src`, `include`, `app`, `tests`,
   * если формат не соответствует `.clang-format`, job падает.

2. **Static analysis (clang-tidy)**:

   * запускается после успешного clang-format,
   * собирает проект (Debug + clang),
   * для каждого исходника запускает `clang-tidy`,
   * любые предупреждения → ошибка CI.

3. **Build & Test (debug)**:

   * использует reusable workflow `build-and-test-reusable.yml`,
   * матрица:

     * ОС: `ubuntu-latest`, `macos-latest`, `windows-latest`,
     * тип сборки: `debug`,
   * собирает и запускает тесты через Meson.

4. **Build & Test (release)**:

   * то же самое, но с `buildtype=release`.

Итого: каждый PR гарантированно:

* отформатирован,
* статически проверен,
* собирается и проходит тесты на всех трёх ОС.

### 7.2. `build-and-test-reusable.yml` — переиспользуемый Workflow

**Тип:** `workflow_call` (его нельзя запустить напрямую через push, только из других workflow’ов).

Параметры:

* `buildtype` — `debug` или `release`;
* `run-tests` — запускать ли `meson test`.

Что делает:

* checkout репозитория;
* вызывает composite action `setup-meson-env`:

  * Python,
  * кэширование pip / meson,
  * установка Meson и Ninja;
* на Windows дополнительно:

  * включает MSVC (через `msvc-dev-cmd`);
* `meson setup` с нужным `buildtype`;
* `meson compile`;
* `meson test` (если `run-tests: true`).

### 7.3. `coverage.yml` — покрытия (Coverage)

**Триггеры:**

* `push` в `main`;
* ручной запуск (`workflow_dispatch`).

Что делает:

1. Checkout.
2. `setup-meson-env` + установка `gcovr`.
3. Сборка с `-Db_coverage=true`:

   * `meson setup build-coverage --buildtype=debug -Db_coverage=true`.
4. `meson test`.
5. `gcovr`:

   * генерирует `coverage.xml` и `coverage.html`.
6. Загружает эти файлы как artifacts (теперь их можно скачать в интерфейсе GitHub Actions).

Зачем:

* видеть, какие части кода реально покрыты тестами;
* помочь принимать решение, куда писать ещё тесты.

### 7.4. `sanitizers.yml` — ASan/UBSan

**Триггеры:**

* `workflow_dispatch` — ручной запуск;
* `schedule` (например, каждый день в 02:00).

Действия:

* checkout;
* установка clang;
* `setup-meson-env`;
* сборка и тесты:

  * ASan:

    ```bash
    CC=clang CXX=clang++ meson setup build-asan --buildtype=debug -Db_sanitize=address
    meson compile -C build-asan
    meson test -C build-asan --print-errorlogs
    ```

  * UBSan:

    ```bash
    CC=clang CXX=clang++ meson setup build-ubsan --buildtype=debug -Db_sanitize=undefined
    meson compile -C build-ubsan
    meson test -C build-ubsan --print-errorlogs
    ```

Зачем:

* ловить «тяжёлые» ошибки работы с памятью и неопределённое поведение, которые не всегда проявляются в обычном CI.

### 7.5. `release.yml` — релизы по тегам (CD)

**Триггер:**

* `push` git-тега вида `v*` (например, `v1.0.0`).

Типичный сценарий:

```bash
git tag v1.0.0
git push origin v1.0.0
```

Что делает workflow:

1. Собирает проект (release) на Linux, macOS и Windows.
2. Запускает тесты.
3. Делает `meson install` в staging-директорию.
4. Упаковывает результат (`tar.gz` для Linux/macOS, `zip` для Windows).
5. Создаёт GitHub Release и прикрепляет артефакты.

Пользователь может зайти во вкладку **Releases** и скачать готовые сборки, не касаясь исходников.

### 7.6. Карта workflow’ов (триггеры → ОС → артефакты)

* `ci.yml`: push/PR → Linux/macOS/Windows → формат, статический анализ, сборка+тесты (debug/release).
* `coverage.yml`: push main / ручной запуск → Linux → coverage.xml + coverage.html (артефакты).
* `sanitizers.yml`: nightly + ручной → Linux (clang) → отчёты в логах ASan/UBSan.
* `release.yml`: тег `v*` → Linux/macOS/Windows → готовые архивы (`tar.gz`/*nix, `zip`/Win) прикладываются к GitHub Release.

---

## 8. Composite Action `setup-meson-env`

Файл: `.github/actions/setup-meson-env/action.yml`.

Зачем:

* чтобы не копировать в каждом job одни и те же шаги:

  * установка Python,
  * кэширование pip/meson,
  * установка Meson+Ninja,
  * загрузка wrap-файла для GoogleTest (gtest), чтобы Meson смог сам подтянуть зависимость на любой ОС.

Пример использования в workflow:

```yaml
- name: Setup Meson env
  uses: ./.github/actions/setup-meson-env
```

Если нужно поменять версию Python или способ установки — достаточно сделать это в **одном месте**.

---

## 9. Как запускать и смотреть результаты CI/CD на GitHub

### 9.1. Автоматические запуски

* Ты пушишь в ветку `main` → запускается `ci.yml` и, при необходимости, `coverage.yml`.
* Ты открываешь Pull Request → запускается `ci.yml`.
* Ты пушишь тег `v1.2.3` → запускается `release.yml`.
* Срабатывает расписание в `sanitizers.yml` → nightly-проверка.

### 9.2. Ручной запуск (workflow_dispatch)

1. Зайди в репозиторий на GitHub.
2. Открой вкладку **Actions**.
3. Выбери нужный workflow (например, **Coverage** или **Sanitizers**).
4. Нажми **Run workflow**.
5. (Опционально) выбери ветку.
6. Нажми **Run workflow**.

### 9.3. Как читать результаты

* В списке workflow runs видно статус:

  * ✅ зелёный — всё ок,
  * ❌ красный — что-то упало.
* Нажми на конкретный запуск → видишь список jobs.
* Нажми на job → видишь шаги, их логи и ошибки.

**Артефакты** (artifacts):

* Внизу страницы run’а есть блок **Artifacts**.
* Примеры:

  * `coverage-report` (с HTML и XML-покрытием).
  * архивы сборок в release workflow.

---

## 10. Как пользоваться этим проектом новичку

### 10.1. Я сделал изменения в коде — что произойдёт?

1. Локально:

   * можешь запустить:

     ```bash
     meson setup build   # при первом запуске
     meson compile -C build
     meson test -C build --print-errorlogs
     ```

2. Когда запушишь ветку или откроешь PR:

   * `ci.yml` запустится автоматически,
   * GitHub покажет статусы (Checks) в PR:

     * format, tidy, debug/release build & test.

Если CI зелёный — изменения «здоровы» с точки зрения сборки и базовых проверок.

### 10.2. У меня упал clang-format — что делать?

1. Посмотри лог job’а `Lint (clang-format)`:

   * там будут файлы, которые не совпадают с ожидаемым форматированием.

2. Локально запусти:

   ```bash
   clang-format -i $(find src include app tests -name '*.cpp' -o -name '*.hpp')
   ```

3. Закоммить изменения и запушь снова.

### 10.3. У меня упал clang-tidy — что делать?

1. Открой лог job’а `Static analysis (clang-tidy)`.
2. Найди конкретное предупреждение и файл/строку.
3. Исправь код в соответствии с рекомендацией (или скорректируй правила, если ты точно прав).
4. Пересобери локально, если нужно.
5. Запушь изменения.

### 10.4. Хочу добавить новый тест

1. Добавь новый `TEST(...)` в `tests/test_calc.cpp` или новый файл в `tests/`.

2. Убедись, что он подключён в `tests/meson.build`.

3. Запусти локально:

   ```bash
   meson test -C build --print-errorlogs
   ```

4. CI автоматически подхватит новый тест.

### 10.5. Хочу добавить ещё одну платформу/конфигурацию

Тебе надо:

* изменить `build-and-test-reusable.yml`:

  * добавить новую ОС в `matrix.os`, например `ubuntu-22.04` или специфичный runner;
* при необходимости — поправить `ci.yml`, если ты создаёшь отдельные jobs.

---

## 11. Как адаптировать этот пример под свой проект

1. Переименовать библиотеку/CLI в `meson.build` и `src/app`.
2. Обновить тесты под свою логику.
3. При желании:

   * скорректировать `.clang-format` под свои требования;
   * изменить набор проверок clang-tidy (список включённых/отключённых проверок);
   * усилить coverage-политику (например, требовать минимум N% покрытия).

Главная идея: **структуру CI/CD можно почти целиком копировать, меняя только названия и параметры сборки.**

---

## 12. Куда двигаться дальше

Идеи для расширения:

* Добавить генерацию документации (Doxygen) и выкладывать её на GitHub Pages.
* Подключить Codecov/ Coveralls и отправлять туда coverage-результаты.
* Добавить интеграционные/системные тесты.
* Добавить линтеры для файлов конфигурации (YAML, JSON, etc).
* Добавить проверку security-зависимостей (Dependabot / специальные экшены).

---

Этот проект задуман как «скелет» для твоих реальных приложений: меняешь код, API и тесты — инфраструктура CI/CD уже готова и работает «как в компании».
