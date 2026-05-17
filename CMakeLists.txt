cmake_minimum_required(VERSION 3.14)
project(AnnePro2Flasher VERSION 1.0 LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Тот самый костыль, который уберет красную ошибку:
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Качаем кроссплатформенный hidapi прямо из исходников
include(FetchContent)
FetchContent_Declare(
    hidapi
    GIT_REPOSITORY https://github.com/libusb/hidapi.git
    GIT_TAG        hidapi-0.14.0
)
FetchContent_MakeAvailable(hidapi)

# Собираем вашу консольную утилиту
add_executable(annepro2_flasher
    src/main.c
    src/annepro2.c
)

# Подключаем библиотеку hidraw (для Линукса)
target_link_libraries(annepro2_flasher PRIVATE hidapi_hidraw)
