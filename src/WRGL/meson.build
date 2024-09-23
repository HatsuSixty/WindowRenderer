WRGL_inc = include_directories('include')

WRGL = library('WRGL', [
  'wrgl.c',
], include_directories : [
  WRGL_inc,
], dependencies : [
  dependency('gl'),
  dependency('egl'),
  dependency('gbm'),
  libWR_dep,
])

WRGL_dep = declare_dependency(link_with : WRGL,
                              include_directories : WRGL_inc)
