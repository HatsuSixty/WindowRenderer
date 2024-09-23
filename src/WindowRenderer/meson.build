window_renderer_inc = include_directories('include')

cc = meson.get_compiler('c')

cmd_args = []

dependencies = [
  dependency('glesv2'),
  dependency('egl'),
  cc.find_library('pthread'),
  cc.find_library('m'),
]

if get_option('desktop_debug_mode')
  dependencies += dependency('glfw3')
  cmd_args += '-DDESKTOP_DEBUG_MODE'
else
  dependencies += dependency('srm')
  dependencies += dependency('libdrm')
endif

executable('WindowRenderer', [
  'renderer/opengl/gl_errors.c',
  'renderer/opengl/texture.c',
  'renderer/opengl/vertex_array.c',
  'renderer/opengl/vertex_buffer.c',
  'renderer/opengl/index_buffer.c',
  'renderer/opengl/shader.c',
  'renderer/renderer.c',
  'session.c',
  'main.c',
  'application.c',
  'server.c',
  'window.c',
], include_directories : [
  window_renderer_inc,
], dependencies : dependencies, c_args : cmd_args)
