# app/meson.build

awesome_calc_cli = executable(
  'awesome_calc_cli',
  'main.cpp',
  dependencies : calc_dep,
  install : true,
)

# Позволяет обращаться к бинарю как к find_program('awesome_calc_cli') в других поддиректориях (tests).
meson.override_find_program('awesome_calc_cli', awesome_calc_cli)
