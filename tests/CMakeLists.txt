
# C++ unit tests, run with `ctest` in the build directory.
#
# Each test is a self-contained executable linking the CSXCAD library that was
# just built, so it exercises the same objects the bindings and applications
# see. Add a new one to TESTS below, no further wiring needed.

set(TESTS
  test_csobject
)

foreach(test ${TESTS})
  add_executable(${test} ${test}.cpp)
  # the tests use the headers of the library they are built with, not the
  # installed ones, which may be older or missing entirely
  target_include_directories(${test} PRIVATE ${CSXCAD_SOURCE_DIR}/src)
  target_link_libraries(${test} CSXCAD)
  add_test(NAME ${test} COMMAND ${test})
endforeach()
