executable('TestClient', [
  'main.c',
], include_directories : [
  shared_inc,
], dependencies : [
  dependency('gl'),
  dependency('egl'),
  dependency('gbm'),
  window_renderer_dep,
  libWR_dep,
  WRGL_dep,
])
