cmake_minimum_required(VERSION 3.4)

set (CMAKE_CXX_STANDARD 11)

project(TG_ThrowItBot)

# set(CMAKE_CXX_FLAGS "-g")

# sources
aux_source_directory(./src SRC_LIST)

# libs
## threads
find_package(Threads REQUIRED)

# zlib
find_package(ZLIB REQUIRED)

## openssl
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

## curl
find_package(CURL)
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()

## boost
set(Boost_USE_MULTITHREADED ON)
if (ENABLE_TESTS)
    find_package(Boost 1.59.0 COMPONENTS system unit_test_framework REQUIRED)
else()
    find_package(Boost 1.59.0 COMPONENTS system REQUIRED)
endif()
include_directories(${Boost_INCLUDE_DIR})

## local
include_directories("/usr/local/include")
link_directories("/usr/local/lib")

## cairo
include(FindPkgConfig)
pkg_check_modules(CAIRO cairo)
if (NOT CAIRO_FOUND)
message(FATAL_ERROR "Cairo not found!")
endif ()
message(STATUS "Cairo library status:")
message(STATUS "    version: ${CAIRO_VERSION}")
message(STATUS "    include path: ${CAIRO_INCLUDE_DIRS}")
message(STATUS "    lib path: ${CAIRO_LIBDIR}")
message(STATUS "    libraries: ${CAIRO_LIBRARIES}")
include_directories(${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBDIR})

## pango
pkg_check_modules(PANGO pangocairo)
if (NOT CAIRO_FOUND)
message(FATAL_ERROR "Pango not found!")
endif ()
message(STATUS "Pango library status:")
message(STATUS "    version: ${PANGO_VERSION}")
message(STATUS "    include path: ${PANGO_INCLUDE_DIRS}")
message(STATUS "    lib path: ${PANGO_LIBDIR}")
message(STATUS "    libs path: ${PANGO_LIBRARY_DIRS}")
message(STATUS "    libraries: ${PANGO_LIBRARIES}")
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBDIR})
link_directories(${PANGO_LIBRARY_DIRS})

## rsvg
pkg_check_modules(RSVG librsvg-2.0)
if (NOT RSVG_FOUND)
message(FATAL_ERROR "rsvg not found!")
endif ()
message(STATUS "rsvg library status:")
message(STATUS "    version: ${RSVG_VERSION}")
message(STATUS "    include path: ${RSVG_INCLUDE_DIRS}")
message(STATUS "    lib path: ${RSVG_LIBDIR}")
message(STATUS "    libs path: ${RSVG_LIBRARY_DIRS}")
message(STATUS "    libraries: ${RSVG_LIBRARIES}")
include_directories(${RSVG_INCLUDE_DIRS})
link_directories(${RSVG_LIBDIR})
link_directories(${RSVG_LIBRARY_DIRS})

# Find OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
include_directories(${OpenCV_INCLUDE_DIRS})

## libwebp
pkg_check_modules(WEBP libwebp)
if (NOT WEBP_FOUND)
message(FATAL_ERROR "libwebp not found!")
endif ()
message(STATUS "libwebp library status:")
message(STATUS "    version: ${WEBP_VERSION}")
message(STATUS "    include path: ${WEBP_INCLUDE_DIRS}")
message(STATUS "    lib path: ${WEBP_LIBDIR}")
message(STATUS "    libraries: ${WEBP_LIBRARIES}")
include_directories(${WEBP_INCLUDE_DIRS})
link_directories(${WEBP_LIBDIR})

set(LIB_LIST
    ${CMAKE_THREAD_LIBS_INIT}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    TgBot
    ${CAIRO_LIBRARIES}
    ${PANGO_LIBRARIES}
    ${RSVG_LIBRARIES}
    ${OpenCV_LIBS}
    ${WEBP_LIBRARIES}
    ArtRobot
)

# building project
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} ${LIB_LIST})

configure_file(picture/p.png p.png COPYONLY)
configure_file(picture/p_mask.png p_mask.png COPYONLY)