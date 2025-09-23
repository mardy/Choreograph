src = [
  'SamplesApp.cpp',
  'pockets/Scene.cpp',
  'samples/BezierConstruction.cpp',
  'samples/Repetition.cpp',
  'samples/SlideAndBounce.cpp',
]

sdl = dependency('sdl2')

executable('SamplesApp',
  sources: src,
  dependencies: [imgui, sdl, choreograph_dep],
)
