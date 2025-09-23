project('Choreograph', 'cpp')

subdir('src/choreograph')
subdir('tests')

imgui = dependency('imgui', required : false)
if imgui.found()
  subdir('samples/src')
endif
