option(CMAKE_MEMTEST "Enable memory testing" OFF)

# If building as a standalone, create the example project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.6)
  project(CSpec)
  add_executable(${PROJECT_NAME}_specs)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}_specs)
else()

endif()

if(DEFINED CMAKE_MEMTEST)
  target_compile_definitions(${PROJECT_NAME}_specs PRIVATE
    "malloc=cspec_malloc"
    "calloc=cspec_calloc"
    "realloc=cspec_realloc"
    "free=cspec_free"
  )
endif()

include_directories(.)

if (WIN32)
  target_compile_definitions(${PROJECT_NAME}_specs PRIVATE
      _CRT_SECURE_NO_WARNINGS
  )
endif()

if (MSVC)
  target_compile_options(${PROJECT_NAME}_specs PRIVATE /W4 /WX /std:clatest)

  # These link warnings get thrown because of cspec_malloc and friends defined
  # in cspec.c as they get referenced by other objs.
  target_link_options(${PROJECT_NAME}_specs PRIVATE /IGNORE:4217,4286)
else()
  target_compile_options(${PROJECT_NAME}_specs PRIVATE
    -Wall -Wextra -Wpedantic -Werror
  )
endif()

target_sources(${PROJECT_NAME}_specs PRIVATE
  cspec.c

  ./tst/test_main.c
  ./tst/cspec_spec.c
)

#target_link_libraries(${PROJECT_NAME}_specs PRIVATE ${PROJECT_NAME}_impl)
