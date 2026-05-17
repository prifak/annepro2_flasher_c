cmake_minimum_required(VERSION 3.14)
project(AnnePro2Flasher VERSION 1.0 LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)
FetchContent_Declare(
    hidapi
    GIT_REPOSITORY https://github.com/libusb/hidapi.git
    GIT_TAG        hidapi-0.14.0
)
FetchContent_MakeAvailable(hidapi)

# making
add_executable(annepro2_flasher
    src/main.c
    src/annepro2.c
)

# set sources directory
target_include_directories(annepro2_flasher PRIVATE ${hidapi_SOURCE_DIR}/hidapi)

# cross-platform usb linking
if (WIN32)
    target_link_libraries(annepro2_flasher PRIVATE hidapi_winapi)
elseif (APPLE)
    target_link_libraries(annepro2_flasher PRIVATE hidapi_darwin)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    target_link_libraries(annepro2_flasher PRIVATE ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY})
else ()
    # for linux
    target_link_libraries(annepro2_flasher PRIVATE hidapi_hidraw)
endif()
