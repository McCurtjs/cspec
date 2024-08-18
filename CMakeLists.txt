# Option for enabling or disabling memory testing
option(CSPEC_MEMTEST "Enable memory testing" OFF)

set(CSPEC_MEMTEST_DEFINES
  "malloc=cspec_malloc"
  "calloc=cspec_calloc"
  "realloc=cspec_realloc"
  "free=cspec_free"
  CACHE INTERNAL "Default set of defines for CSpec memory testing"
)

add_library(CSpec)
target_sources(CSpec PRIVATE cspec.c)
target_include_directories(CSpec PUBLIC ./)

if(CSPEC_MEMTEST STREQUAL ON)
  target_compile_definitions(CSpec PUBLIC ${CSPEC_MEMTEST_DEFINES})

  if(MSVC)
    # These link warnings get thrown because of cspec_malloc and friends defined
    # in cspec.c as they get referenced by other objs.
    target_link_options(${PROJECT_NAME}_specs PUBLIC /IGNORE:4217,4286)
  endif()
endif()

# If building as a standalone, create the example project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.6)
  project(CSpec)
  add_executable(${PROJECT_NAME}_specs)
  target_link_libraries(${PROJECT_NAME}_specs PRIVATE CSpec)
  target_sources(${PROJECT_NAME}_specs PRIVATE
  ./tst/test_main.c
  ./tst/cspec_spec.c
  )

  if (MSVC)
    target_compile_options(${PROJECT_NAME}_specs PRIVATE /W4 /WX /std:clatest)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}_specs)
  else()
    target_compile_options(${PROJECT_NAME}_specs PRIVATE
      -Wall -Wextra -Wpedantic -Werror
    )
  endif()
endif()
