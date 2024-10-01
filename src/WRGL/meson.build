WRGL_inc = include_directories('include')

WRGL = library('WRGL', [
  'wrgl_buffer.c',
  'wrgl_context.c',
  'wrgl.c',
  'glext.c',
], include_directories : [
  WRGL_inc,
  shared_inc,
], dependencies : [
  dependency('gl'),
  dependency('egl'),
  dependency('gbm'),
  window_renderer_dep,
  libWR_dep,
])

WRGL_dep = declare_dependency(link_with : WRGL,
                              include_directories : WRGL_inc)
