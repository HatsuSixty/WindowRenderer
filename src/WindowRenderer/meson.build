window_renderer_inc = include_directories('include')

cc = meson.get_compiler('c')

executable('WindowRenderer', [
  'input_events/mouse.c',
  'renderer/opengl/gl_errors.c',
  'renderer/opengl/texture.c',
  'renderer/opengl/vertex_array.c',
  'renderer/opengl/vertex_buffer.c',
  'renderer/opengl/index_buffer.c',
  'renderer/opengl/shader.c',
  'renderer/renderer.c',
  'renderer/glext.c',
  'server/session.c',
  'server/server.c',
  'server/window.c',
  'application.c',
  'input.c',
  'main.c',
  'wm.c',
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
