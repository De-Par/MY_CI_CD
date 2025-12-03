project(
  'awesome_calc',
  'cpp',
  version : '1.1.0',
  default_options : [
    'cpp_std=c++17',
    'warning_level=3',
    'werror=true',
    'buildtype=debugoptimized',
  ]
)

# Библиотека объявляется здесь, чтобы её видели и app/, и tests/
calc_includes = include_directories('include')

calc_lib = library(
  'awesome_calc',
  'src/calc.cpp',
  include_directories : calc_includes,
  install : true,
)

calc_dep = declare_dependency(
  link_with : calc_lib,
  include_directories : calc_includes,
)

subdir('app')
subdir('tests')

# Локальные цели для форматирования (если clang-format установлен в системе).
clang_format = find_program('clang-format', required : false)

if clang_format.found()
  format_files = [
    'app/main.cpp',
    'include/awesome_calc/calc.hpp',
    'src/calc.cpp',
    'tests/test_calc.cpp',
  ]

  run_target(
    'format-check',
    command : [clang_format, '--dry-run', '--Werror'] + format_files,
  )

  run_target(
    'format',
    command : [clang_format, '-i'] + format_files,
  )
endif
