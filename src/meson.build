calc_includes = include_directories('../include')

calc_lib = library(
  'awesome_calc',
  'calc.cpp',
  include_directories : calc_includes,
  install : true,
)

calc_dep = declare_dependency(
  link_with : calc_lib,
  include_directories : calc_includes
)
