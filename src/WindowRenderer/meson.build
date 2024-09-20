window_renderer_inc = include_directories('include')

cc = meson.get_compiler('c')

executable('WindowRenderer', [
  'opengl/gl_errors.c',
  'opengl/texture.c',
  'opengl/vertex_array.c',
  'opengl/vertex_buffer.c',
  'opengl/index_buffer.c',
  'opengl/shader.c',
  'main.c',
  'application.c',
  'server.c',
  'window.c',
], dependencies : [
  dependency('srm'),
  dependency('glesv2'),
  dependency('libdrm'),
  cc.find_library('pthread'),
  cc.find_library('m'),
], include_directories : [
  window_renderer_inc,
])
