executable('TestClient', [
  'main.c',
  'server.c',
  'window.c',
  'server_session.c',
], include_directories : [
  window_renderer_inc,
], dependencies : [
  dependency('gl'),
  dependency('egl'),
  dependency('gbm'),
])
