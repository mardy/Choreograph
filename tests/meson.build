tests = [
  'Benchmarks_test',
  'Cue_test',
  'Ease_test',
  'ForumMiscellany_test',
  'Grouping_test',
  'Motion_test',
  'Numbers_test',
  'Phrase_test',
  'Sequence_test',
  'Timeline_test',
]

foreach t: tests
  exe = executable(t,
    t + '.cpp',
    override_options: [
      'cpp_std=c++14',
    ],
    dependencies: [
      choreograph_dep,
    ])
  test(t, exe)
endforeach
