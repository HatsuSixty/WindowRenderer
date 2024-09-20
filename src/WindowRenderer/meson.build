window_renderer_inc = include_directories('include')

cc = meson.get_compiler('c')

executable('WindowRenderer', [
  'renderer/opengl/gl_errors.c',
  'renderer/opengl/texture.c',
  'renderer/opengl/vertex_array.c',
  'renderer/opengl/vertex_buffer.c',
  'renderer/opengl/index_buffer.c',
  'renderer/opengl/shader.c',
  'renderer/renderer.c',
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
