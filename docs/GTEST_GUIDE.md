# GTest: краткий гайд для проекта awesome_calc

Документ для быстрого старта: базовая теория, используемые конструкции в проекте, примеры, и что ещё можно применять, чтобы писать тесты средней сложности.

---

## 1. Что такое GTest и зачем он нам

GoogleTest (GTest) — популярный фреймворк для юнит- и интеграционного тестирования C++. В нашем проекте:

- Юнит-тесты (`tests/unit/calc_unit_test.cpp`) проверяют функции библиотеки `awesome_calc`.
- Интеграционный тест (`tests/integration/cli_integration_test.cpp`) запускает CLI и проверяет вывод.
- Meson собирает отдельные тестовые бинарники и гоняет их через `meson test`.

---

## 2. Базовые конструкции GTest

### 2.1. Макросы проверок

- `EXPECT_EQ(a, b)`, `ASSERT_EQ(a, b)` — равенство. `ASSERT_*` прекращает тест при провале, `EXPECT_*` — продолжает.
- `EXPECT_DOUBLE_EQ`, `EXPECT_NEAR` — для чисел с плавающей точкой.
- `EXPECT_THROW(expr, ExceptionType)` — ожидаемое исключение.
- `EXPECT_TRUE/EXPECT_FALSE`, `EXPECT_LT/LE/GT/GE/NE` — логические сравнения.

### 2.2. Тест-кейсы без фикстур

```cpp
TEST(SuiteName, TestName) {
    EXPECT_EQ(add(1, 2), 3);
}
```

### 2.3. Фикстуры (общая подготовка/сброс)

```cpp
class MyFixture : public ::testing::Test {
protected:
    void SetUp() override { /* подготовка */ }
    void TearDown() override { /* очистка */ }
};

TEST_F(MyFixture, Works) {
    // есть доступ к полям фикстуры
}
```

### 2.4. Параметризованные тесты

```cpp
using Params = std::tuple<int, int, int>;
class MyParamTest : public ::testing::TestWithParam<Params> {};

TEST_P(MyParamTest, HandlesCases) {
    auto [a, b, expected] = GetParam();
    EXPECT_EQ(add(a, b), expected);
}

INSTANTIATE_TEST_SUITE_P(
    Cases,
    MyParamTest,
    ::testing::Values(
        Params{1, 2, 3},
        Params{-1, 1, 0}
    ));
```

### 2.5. Собственный `main`

Если не используете `gtest_main`, добавьте:

```cpp
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

---

## 3. Как мы применяем GTest в проекте

- **Юнит** (`tests/unit/calc_unit_test.cpp`):
  - Параметризованный тест для `clamp_add` (покрытие разных границ).
  - Фикстура `RunningStatsTest` с `SetUp` для stateful API.
  - Проверки исключений (`EXPECT_THROW`) для невалидных входов.
- **Интеграция** (`tests/integration/cli_integration_test.cpp`):
  - Собственный `main` (без gtest_main).
  - Вспомогательная функция запуска CLI и чтения stdout.
  - Проверка строк выхода (end-to-end сценарий).
- **Smoke**: отдельный Meson test `cli-smoke` просто запускает бинарь.

---

## 4. Как запускать тесты

```bash
meson test -C build --print-errorlogs     # все тесты
meson test -C build --suite unit          # только юнит
meson test -C build --suite integration   # только интеграция
```

---

## 5. Полезные паттерны для “средних” тестов

1) **Матчеры (GMock)**  
   Можно использовать matchers из gmock даже без моков: `EXPECT_THAT(vec, ::testing::ElementsAre(1,2,3));`. Это делает проверки коллекций и строк читабельнее.

2) **Typed tests**  
   Один тест для разных типов: полезно, если появятся шаблонные функции. Пример:
   ```cpp
   template <typename T>
   class MeanTest : public ::testing::Test {};
   using MyTypes = ::testing::Types<float, double>;
   TYPED_TEST_SUITE(MeanTest, MyTypes);

   TYPED_TEST(MeanTest, ComputesMean) {
       std::vector<TypeParam> v{1,2,3};
       EXPECT_NEAR(mean(v), static_cast<TypeParam>(2), 1e-6);
   }
   ```

3) **Моки**  
   Когда появятся зависимости (например, ввод/вывод, сеть), можно замокать интерфейсы через gmock (`MockFoo`, `EXPECT_CALL`). Сейчас в проекте не используется.

4) **Death tests**  
   Для проверки, что код падает/abort в ожидании (используйте осторожно, требуют отдельного бинаря или опции `--gtest_death_test_style=threadsafe`).

5) **`SetUpTestSuite`/`TearDownTestSuite`**  
   Если нужно тяжёлое разделяемое состояние на весь suite (например, временный каталог или объект), а не на каждый тест.

6) **Кастомные матчеры**  
   Можно написать свой `MATCHER`/`MATCHER_P`, если нужна читабельность в проверках сложных структур.

---

## 6. Советы по оформлению тестов

- Один suite — один аспект поведения: `MeanTest`, `MedianTest`, `ClampAddTest`, `RunningStatsTest`.
- Имена тестов — что проверяем (`ThrowsOnEmpty`, `SaturatesWithinBounds`).
- Включайте негативные кейсы (исключения, границы) и позитивные (корректный расчёт).
- Для stateful-API используйте фикстуры и `SetUp`/`TearDown`.
- Для наборов кейсов — параметризованные тесты вместо дублирования кода.
- Проверки вывода/строк — matchers `HasSubstr`/`ElementsAre` (можно добавить в интеграционные тесты).

---

## 7. Шаблон для нового юнит-теста (быстрый старт)

```cpp
#include <gtest/gtest.h>
#include "awesome_calc/calc.hpp"

class MyFeatureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // подготовка
    }
};

TEST_F(MyFeatureTest, WorksForHappyPath) {
    EXPECT_EQ(awesome_calc::add(2, 2), 4);
}

TEST_F(MyFeatureTest, ThrowsOnBadInput) {
    EXPECT_THROW(awesome_calc::mean({}), std::invalid_argument);
}
```

---

## 8. Что ещё почитать

- Официально: https://google.github.io/googletest/
- Matchers (gmock): https://google.github.io/googletest/reference/matchers.html
- Death tests: https://google.github.io/googletest/reference/testing.html#death-test-support
- Примеры параметризованных и типизированных тестов: https://google.github.io/googletest/advanced.html
