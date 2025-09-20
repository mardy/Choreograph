src = [
  'Cue.cpp',
  'Timeline.cpp',
  'TimelineItem.cpp',
]

choreograph = library('choreograph',
  sources: src,
  include_directories: '..',
  install: true,
)

choreograph_dep = declare_dependency(
  link_with: choreograph,
  include_directories: '..',
)
