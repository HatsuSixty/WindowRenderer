libWR_inc = include_directories('include')

libWR = library('libWR', [
  'server.c',
  'server_session.c',
  'window.c',
], include_directories : [
  libWR_inc,
], dependencies : [
  dependency('gl'),
  dependency('egl'),
  dependency('gbm'),
  window_renderer_dep,
])

libWR_dep = declare_dependency(link_with : libWR,
                               include_directories : libWR_inc)
