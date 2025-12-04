project(
  'awesome_calc',
  'cpp',
  version : '1.1.0',
  default_options : [
    'cpp_std=c++17',
    'warning_level=3',
    'werror=true',
    'buildtype=debugoptimized',
    'default_library=both',
  ]
)

# Библиотека объявляется здесь, чтобы её видели и app/, и tests/
calc_includes = include_directories('include')

calc_cpp_args = []
calc_dep_args = []
calc_is_shared = get_option('default_library') != 'static'

if host_machine.system() == 'windows' and calc_is_shared
  # Для shared на Windows нужны __declspec(dllexport/dllimport).
  calc_cpp_args += ['-DAWESOME_CALC_SHARED', '-DAWESOME_CALC_BUILD']
  calc_dep_args += ['-DAWESOME_CALC_SHARED']
endif

calc_lib = library(
  'awesome_calc',
  'src/calc.cpp',
  cpp_args : calc_cpp_args,
  include_directories : calc_includes,
  install : true,
)

calc_dep = declare_dependency(
  link_with : calc_lib,
  include_directories : calc_includes,
  compile_args : calc_dep_args,
)

subdir('app')
subdir('tests')

# Локальные цели для форматирования (если clang-format установлен в системе).
clang_format_env = meson.get_env('CLANG_FORMAT')
if clang_format_env != ''
  clang_format = find_program(clang_format_env, required : false)
else
  clang_format = find_program('clang-format-21', required : false)
  if not clang_format.found()
    clang_format = find_program('clang-format-15', required : false)
    if not clang_format.found()
      clang_format = find_program('clang-format', required : false)
    endif
  endif
endif

if clang_format.found()
  format_files = [
    'app/main.cpp',
    'include/awesome_calc/calc.hpp',
    'src/calc.cpp',
    'tests/unit/calc_unit_test.cpp',
    'tests/integration/cli_integration_test.cpp',
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
