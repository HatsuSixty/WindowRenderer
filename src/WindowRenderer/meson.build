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
  'renderer/glext.c',
  'input_events/mouse.c',
  'input.c',
  'session.c',
  'main.c',
  'application.c',
  'server.c',
  'window.c',
  'log.c',
], include_directories : [
  window_renderer_inc,
  shared_inc,
], dependencies : [
  cc.find_library('pthread'),
  cc.find_library('m'),
  dependency('srm'),
  dependency('libdrm'),
  dependency('glesv2'),
  dependency('egl'),
])

window_renderer_dep = declare_dependency(include_directories : window_renderer_inc)
