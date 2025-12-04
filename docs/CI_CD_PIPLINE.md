# CI/CD пайплайн проекта awesome_calc

Документ для начинающих: что за пайплайн, как он устроен, что запускается, зачем и как управлять.

---

## 1. Общая картина

- **CI** (Continuous Integration): форматирование, статический анализ, сборки и тесты на Linux/macOS/Windows при push/PR в `main`.
- **Доп. проверки**: coverage (gcovr), санитайзеры (ASan/UBSan/TSan).
- **CD** (Continuous Delivery): сборка и публикация релизов по тегам `v*`, вручную или по расписанию, с артефактами для всех трёх ОС.
- **Окружение**: Meson+Ninja, GoogleTest через wrap, clang-format/clang-tidy. Библиотека собирается как static и shared; Windows экспортируется через `AWESOME_CALC_API`.
- **Управление**: матрицы ОС и режимы CD настраиваются в YAML; есть переменная `CD_MODE`; можно использовать self-hosted Linux раннеры.

Справка по аббревиатуре:
- **CI** — Continuous Integration.
- **CD** — Continuous Delivery.
- **ASan** — AddressSanitizer (ошибки памяти: UAF, выход за границы).
- **UBSan** — UndefinedBehaviorSanitizer (неопределённое поведение).
- **TSan** — ThreadSanitizer (гонки потоков).
- **PR** — Pull Request.
- **OS** — Operating System (ОС).

### 1.1. Визуализация: от кода до продакшена

```mermaid
flowchart LR
  A[Код/commit] --> B{push или PR в main}
  B --> C[CI: форматирование, tidy, build+test\n(Linux/macOS/Windows)]
  C --> D[Доп. проверки: coverage + sanitizers]
  D --> E{CI зелёный?}
  E -->|нет| A
  E -->|да| F[tag v* или Run Release]
  F --> G[Release: build/test/install\n tar.gz (Linux/macOS), zip (Windows)]
  G --> H[GitHub Release\n + артефакты]
  H --> I[Готовые сборки для скачивания]
```

### 1.2. Визуализация: какие workflow’ы когда запускаются

```mermaid
flowchart TB
  subgraph Triggers
    P[push/PR main]
    TAG[tag v*]
    MAN[workflow_dispatch]
    CRON[schedule cron]
  end

  P --> CI[ci.yml\n(матрица OS)]
  P --> COV[coverage.yml\n(Linux)]
  P --> SAN[sanitizers.yml\n(Linux)]

  CRON --> SAN
  MAN --> SAN
  MAN --> COV

  TAG --> REL[release.yml\n(CD)]
  MAN --> REL
  CRON --> REL:::maybe

  CI --> OK[зелёный статус]
  REL --> ART[tar.gz/zip артефакты в Release]

  classDef maybe fill:#f9f9f9,stroke:#bbb;
```

---

## 2. Структура workflow’ов

Все файлы лежат в `.github/workflows/`:

- `ci.yml`: основной CI (push/PR → main), матрица Linux/macOS/Windows.
- `build-test-reusable.yml`: переиспользуемый workflow для сборки/тестов.
- `coverage.yml`: coverage (Linux).
- `sanitizers.yml`: ASan/UBSan/TSan (Linux, по расписанию/ручной/push main).
- `release.yml`: CD по тегам `v*`, по расписанию или вручную (`workflow_dispatch`), с управлением режимом через `CD_MODE`.

Дополнительно:
- `.github/actions/setup-meson-env`: композитный action (Python+Meson+Ninja, кеш, gtest wrap).

---

## 3. Основной CI (`ci.yml`)

- Триггеры: `push`/`pull_request` в `main`.
- Jobs:
  - `Lint: clang-format` → устанавливает clang-format и проверяет все `*.cpp/*.hpp` в src/include/app/tests.
  - `Analyze: clang-tidy` → clang + clang-tidy, Meson setup, build, tidy с `--warnings-as-errors`.
  - `Build/Test: debug` и `Build/Test: release` → вызывают `build-test-reusable.yml` с матрицей ОС, сборка + `meson test`.
- Матрица ОС: `[ubuntu-latest, macos-latest, windows-latest]`. Можно убрать/добавить ОС, либо условно выключать через repo variables (`if: runner.os != 'Windows' || vars.ENABLE_WINDOWS == 'true'`).

---

## 4. Переиспользуемый workflow (`build-test-reusable.yml`)

- Тип: `workflow_call`.
- Входы: `buildtype` (debug/release), `run-tests` (bool).
- Шаги:
  - checkout
  - setup Meson env (композитный action)
  - msvc-dev-cmd (Windows)
  - `meson setup builddir --buildtype=...`
  - `meson compile -C builddir`
  - `meson test -C builddir` (если `run-tests=true`)
- Используется в CI и может вызываться из других workflow’ов.

---

## 5. Coverage (`coverage.yml`)

- Триггеры: push в `main`, `workflow_dispatch`.
- Шаги: checkout → setup Meson env → `pip install gcovr` → `meson setup build-coverage --buildtype=debug -Db_coverage=true` → build → test → `gcovr` → загрузка артефактов `coverage.xml`/`coverage.html`.
- ОС: Linux. Это быстрее/надёжнее, чем мультиплатформенно.

---

## 6. Санитайзеры (`sanitizers.yml`)

- Триггеры: `workflow_dispatch`, `push` в `main`, `schedule` (cron).
- Матрица: `address` / `undefined` / `thread`.
- Конфигурация: `CC=clang CXX=clang++ meson setup build-<kind> --buildtype=debug -Db_sanitize=<kind> -Db_lundef=false` → build → test.
- ОС: Linux (clang). На macOS поддержка частичная, на Windows TSan нет, поэтому Linux-only.
- Названия чеков: `Sanitizer (address|undefined|thread)`.

---

## 7. Релизы / CD (`release.yml`)

- Триггеры: `push` тега `v*`, `workflow_dispatch` (ручной), `schedule` (если включено).
- Режимы через repo variable `CD_MODE`:
  - `auto` (по умолчанию): теги + ручной.
  - `scheduled`: cron + ручной.
  - `manual`: только ручной.
- Проверка зелёного CI при ручном запуске: опция `require-ci-success`.
- Матрица ОС: Linux/macOS/Windows (настраивается в `matrix.os`).
- Шаги: Meson setup (release) → build → test → `meson install` → упаковка (`tar.gz` для *nix, `zip` для Windows) → GitHub Release (softprops/action-gh-release).

---

## 8. Self-hosted Linux

Как гонять CI/CD на своём сервере:
1. Repo Settings → Actions → Runners → New self-hosted runner → Linux → следовать инструкции (`./config.sh`, token).
2. Запустить агент (`./run.sh` или как сервис).
3. В workflow заменить `runs-on: ubuntu-latest` на `runs-on: self-hosted` (или `['self-hosted','linux']` если использовали label).
4. macOS/Windows можно оставить на hosted или поднять свои раннеры соответствующей ОС.

---

## 9. Тестовый набор

- Юнит (GTest): `tests/unit/calc_unit_test.cpp`.
- Интеграция (GTest): `tests/integration/cli_integration_test.cpp` (проверяет stdout CLI).
- Smoke: `cli-smoke` (Meson test запускает бинарь).
- Санитайзеры: отдельный workflow.
- Coverage: отдельный workflow.
Запуск локально: `meson test -C build` (гоняет unit + integration + smoke).

---

## 10. Форматирование и статический анализ

- clang-format: цель `format`/`format-check` в `meson.build`; CI job `Lint: clang-format`; скрипт `tools/clang-format-all.sh` (обходит `src/include/app/tests`, показывает изменённые файлы с 🟡 или пишет, что всё ок).
- clang-tidy: job `Analyze: clang-tidy`, падает на любых предупреждениях (`--warnings-as-errors`).

---

## 11. Как управлять набором ОС и тяжёлых проверок

- Матрица ОС в `ci.yml` и `release.yml` → редактируйте `matrix.os` или добавьте условия `if:` с repo variables.
- Санитайзеры/coverage оставлены Linux-only для скорости и стабильности; можно расширить, но потребуются настройки и время.
- CD режимы через `CD_MODE` (`auto`/`scheduled`/`manual`).

---

## 12. Быстрые рецепты (для практики)

- Локальная сборка/тесты:
  ```bash
  meson setup build
  meson compile -C build
  meson test -C build --print-errorlogs
  ```
- Запуск санитайзера (локально, clang):
  ```bash
  CC=clang CXX=clang++ meson setup build-asan --buildtype=debug -Db_sanitize=address -Db_lundef=false
  meson test -C build-asan --print-errorlogs
  ```
- Форматирование:
  ```bash
  meson compile -C build format    # или
  ./tools/clang-format-all.sh
  ```
- Выпуск релиза вручную:
  1) Убедитесь, что CI зелёный.
  2) Actions → Release → Run workflow → укажите ref/требование CI → Run.
  3) Заберите артефакты в GitHub Release.

---

## 13. Что можно расширить

- Добавить TSan/ASan на macOS (дороже и сложнее), ASan для clang-cl на Windows.
- Добавить Codecov/Coveralls с загрузкой `coverage.xml`.
- Добавить линтеры для YAML/JSON (yamllint/prettier) как отдельный job.
- Добавить Doxygen/Docs → GitHub Pages.
- Добавить pre-commit hooks для clang-format/clang-tidy.
