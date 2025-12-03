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
