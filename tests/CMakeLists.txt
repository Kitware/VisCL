project(viscl_tests)

enable_testing()
include(CTest)

set(test_output_path
  "${viscl_BINARY_DIR}/bin")
set(test_base_output_path
  "${test_output_path}")
set(test_working_path
  "${viscl_BINARY_DIR}/tests")

add_custom_target(tests)

function (viscl_declare_test testname)
  if (NOT WIN32)
    add_custom_target(tests-${testname})
    add_dependencies(tests
      tests-${testname})
  endif (NOT WIN32)
endfunction (viscl_declare_test)

macro (viscl_build_test testname libraries)
  add_executable(test-${testname} ${ARGN})
  set_target_properties(test-${testname}
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${test_base_output_path}")
  target_link_libraries(test-${testname}
    ${${libraries}})
  viscl_declare_test(${testname})
endmacro (viscl_build_test)

function (viscl_make_test testname instance)
  add_test(
    NAME    test-${testname}-${instance}
    COMMAND ${test_runner}
            "${test_output_path}/test-${testname}"
            ${instance}
            ${ARGN})
  set_tests_properties(test-${testname}-${instance}
    PROPERTIES
      WORKING_DIRECTORY       "${test_working_path}"
      FAIL_REGULAR_EXPRESSION "^Error: ;\nError: "
      REQUIRED_FILES          "${test_output_path}/test-${testname}")

  if (NOT WIN32)
    add_custom_target(test-${testname}-${instance})
    add_custom_command(
      TARGET  test-${testname}-${instance}
      COMMAND ${test_environment}
              ${test_runner}
              "${test_output_path}/test-${testname}"
              ${instance}
              ${ARGN}
      WORKING_DIRECTORY
              "${test_working_path}"
      COMMENT "Running test \"${testname}\" instance \"${instance}\"")
    add_dependencies(tests-${testname}
      test-${testname}-${instance})
  endif (NOT WIN32)
endfunction(viscl_make_test)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")


add_subdirectory(core)
#add_subdirectory(tasks)
