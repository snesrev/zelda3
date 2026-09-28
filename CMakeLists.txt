cmake_minimum_required(VERSION 3.24)

project(Zelda3 LANGUAGES C OBJC)

if(NOT APPLE OR NOT CMAKE_SYSTEM_NAME MATCHES "tvOS")
  message(FATAL_ERROR "This CMake project is for tvOS. Configure with -DCMAKE_SYSTEM_NAME=tvOS -DCMAKE_OSX_SYSROOT=appletvos.")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum supported tvOS version")

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL_AUDIO ON CACHE BOOL "" FORCE)
set(SDL_JOYSTICK ON CACHE BOOL "" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)

set(SDL2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2")
set(SDL2_TVOS_PATCH "${CMAKE_CURRENT_SOURCE_DIR}/platform/tvos/SDL2-tvos-scene.patch")
if(NOT EXISTS "${SDL2_SOURCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "SDL2 submodule is missing. Run: git submodule update --init --recursive")
endif()
find_program(GIT_EXECUTABLE git REQUIRED)
execute_process(
  COMMAND "${GIT_EXECUTABLE}" -C "${SDL2_SOURCE_DIR}" apply --check --whitespace=nowarn "${SDL2_TVOS_PATCH}"
  RESULT_VARIABLE SDL2_PATCH_CHECK
  OUTPUT_QUIET
  ERROR_QUIET)
if(SDL2_PATCH_CHECK EQUAL 0)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${SDL2_SOURCE_DIR}" apply --whitespace=nowarn "${SDL2_TVOS_PATCH}"
    RESULT_VARIABLE SDL2_PATCH_APPLY)
  if(NOT SDL2_PATCH_APPLY EQUAL 0)
    message(FATAL_ERROR "Failed to apply the tvOS SDL2 scene-lifecycle patch")
  endif()
else()
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${SDL2_SOURCE_DIR}" apply --reverse --check --whitespace=nowarn "${SDL2_TVOS_PATCH}"
    RESULT_VARIABLE SDL2_PATCH_REVERSE_CHECK
    OUTPUT_QUIET
    ERROR_QUIET)
  if(NOT SDL2_PATCH_REVERSE_CHECK EQUAL 0)
    message(FATAL_ERROR "SDL2 is neither pristine nor patched. Restore the submodule before configuring.")
  endif()
endif()
add_subdirectory(third_party/SDL2 EXCLUDE_FROM_ALL)

file(GLOB ZELDA3_SOURCES CONFIGURE_DEPENDS
  src/*.c
  snes/*.c
)

list(REMOVE_ITEM ZELDA3_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/opengl.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/glsl_shader.c"
)
list(APPEND ZELDA3_SOURCES
  third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c
  src/platform/tvos/opengl_stub.c
)

add_executable(zelda3 MACOSX_BUNDLE ${ZELDA3_SOURCES})
target_include_directories(zelda3 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(zelda3 PRIVATE __TVOS__)
target_link_libraries(zelda3 PRIVATE SDL2::SDL2main SDL2::SDL2-static)

set_target_properties(zelda3 PROPERTIES
  OUTPUT_NAME "Zelda3"
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platform/tvos/Info.plist"
  XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "App Icon"
  XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.snesrev.zelda3"
  XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "3"
  XCODE_ATTRIBUTE_TVOS_DEPLOYMENT_TARGET "${CMAKE_OSX_DEPLOYMENT_TARGET}"
  XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
  XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
)

set(TVOS_ASSET_CATALOG "${CMAKE_CURRENT_SOURCE_DIR}/platform/tvos/Assets.xcassets")
target_sources(zelda3 PRIVATE "${TVOS_ASSET_CATALOG}")
set_source_files_properties("${TVOS_ASSET_CATALOG}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set(TVOS_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform/tvos/Resources")
set(TVOS_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/zelda3.ini")
foreach(resource zelda3_assets.dat zelda3.sfc zelda3.smc)
  if(EXISTS "${TVOS_RESOURCE_DIR}/${resource}")
    list(APPEND TVOS_RESOURCES "${TVOS_RESOURCE_DIR}/${resource}")
  elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${resource}")
    list(APPEND TVOS_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${resource}")
  endif()
endforeach()
target_sources(zelda3 PRIVATE ${TVOS_RESOURCES})
set_source_files_properties(${TVOS_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ZELDA3_SOURCES} ${TVOS_RESOURCES})

message(STATUS "tvOS resources: ${TVOS_RESOURCES}")
if(NOT EXISTS "${TVOS_RESOURCE_DIR}/zelda3.sfc" AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/zelda3.sfc")
  message(WARNING "No ROM bundled. Put the authorized US ROM at platform/tvos/Resources/zelda3.sfc and reconfigure before installing.")
endif()
if(NOT EXISTS "${TVOS_RESOURCE_DIR}/zelda3_assets.dat" AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/zelda3_assets.dat")
  message(WARNING "No extracted assets bundled. Run the asset extraction command in platform/tvos/README.md and reconfigure.")
endif()
