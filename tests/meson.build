# tests/meson.build

# Найти GoogleTest. Если используешь wrap:
# meson wrap install gtest
gtest_dep = dependency('gtest', main : true, required : true)

test_exe = executable(
  'unit_tests',
  'test_calc.cpp',
  dependencies : [gtest_dep, calc_dep],
)

test(
  'unit-tests',
  test_exe
)

# Запускаем готовый CLI как smoke-тест: проверяем, что бинарь запускается и пишет ожидаемый вывод.
cli_prog = find_program('awesome_calc_cli')
test(
  'cli-smoke',
  cli_prog,
  args : [],
  should_fail : false,
  timeout : 10,
)
