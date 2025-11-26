#add_executable (test-client test-client.cpp)
#target_include_directories (test-client PUBLIC
#  ${CMAKE_CURRENT_SOURCE_DIR}/../include
#  ${CMAKE_CURRENT_BINARY_DIR}/../src
#)
#target_link_libraries (test-client PRIVATE Catch2::Catch2WithMain alpha::server alpha::client)
#add_test (NAME TestClient COMMAND ./test-client)

file (GLOB test-sources ${CMAKE_CURRENT_SOURCE_DIR}/test-*.cpp)

foreach (test-source ${test-sources})
    get_filename_component (name ${test-source} NAME_WE)
    add_executable (alpha-${name} ${test-source})
    target_link_libraries (alpha-${name}
        PRIVATE Catch2::Catch2WithMain alpha::client alpha::server coverage_config)
    add_test (NAME alpha-${name} COMMAND ./alpha-${name})
    set_property (TEST alpha-${name} PROPERTY
                  ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}")
    install (TARGETS alpha-${name}
             DESTINATION bin)
endforeach ()

if (ENABLE_PYTHON)

    set (PYTHON_MODULE_DIR ${CMAKE_SOURCE_DIR}/python)
    file (GLOB_RECURSE PYTHON_TEST_FILES "${PYTHON_MODULE_DIR}/test_*.py")

    foreach (PYTHON_TEST_FILE ${PYTHON_TEST_FILES})
        file (RELATIVE_PATH PYTHON_TEST_FILE_REL ${PYTHON_MODULE_DIR} ${PYTHON_TEST_FILE})
        string (REPLACE ".py" "" PYTHON_TEST_FILE_BASE ${PYTHON_TEST_FILE_REL})
        string (REPLACE "/" "." PYTHON_TEST_NAME ${PYTHON_TEST_FILE_BASE})
        if (${ENABLE_COVERAGE})
            add_test (NAME ${PYTHON_TEST_NAME} COMMAND coverage run -a -m unittest ${PYTHON_TEST_NAME})
        else ()
            add_test (NAME ${PYTHON_TEST_NAME} COMMAND ${Python_EXECUTABLE} -m unittest ${PYTHON_TEST_NAME})
        endif ()
        set_property (TEST ${PYTHON_TEST_NAME} PROPERTY ENVIRONMENT
                      PYTHONPATH=${CMAKE_SOURCE_DIR}/python/:${CMAKE_BINARY_DIR}/python:$ENV{PYTHONPATH})
    endforeach ()

endif ()
