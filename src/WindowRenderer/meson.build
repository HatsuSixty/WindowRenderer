window_renderer_inc = include_directories('include')

cc = meson.get_compiler('c')

executable('WindowRenderer', [
  'main.c',
  'server.c',
  'window.c',
], dependencies : [
  dependency('raylib'),
  cc.find_library('pthread'),
], include_directories : [
  window_renderer_inc,
])
