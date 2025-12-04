# tests/meson.build

# Найти GoogleTest. Если используешь wrap:
# meson wrap install gtest
gtest_dep = dependency('gtest', main : true, required : true)

# Unit тесты — изолированно проверяют API.
unit_tests = executable(
  'unit_tests',
  ['unit/calc_unit_test.cpp'],
  dependencies : [gtest_dep, calc_dep],
)

test(
  'unit-tests',
  unit_tests,
  suite : 'unit'
)

# Интеграционные тесты — гоняют готовый CLI и проверяют связку целиком.
integration_tests = executable(
  'integration_tests',
  ['integration/cli_integration_test.cpp'],
  dependencies : [gtest_dep, calc_dep],
)

test(
  'integration-tests',
  integration_tests,
  suite : 'integration',
  env : {
    'MESON_BUILD_ROOT' : meson.project_build_root(),
  }
)

# Smoke-тест CLI — просто запускает бинарь, чтобы убедиться, что он стартует.
cli_prog = find_program('awesome_calc_cli')
test('cli-smoke', cli_prog, args : [], should_fail : false, timeout : 10)
