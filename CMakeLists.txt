cmake_minimum_required (VERSION 3.8)

project ("ai-tool")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/extern/)

if ( NOT ("${HAS_JSON}"))
    message("[ai tool] adding json project")
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/json ${CMAKE_SOURCE_DIR}/build/json)
endif()

if ( NOT ("${HAS_ARRAY}"))
    message("[ai tool] adding array project")
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/array ${CMAKE_SOURCE_DIR}/build/array)
endif()

if ( NOT ("${HAS_DICT}"))
    message("[ai tool] adding dict project")
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/dict ${CMAKE_SOURCE_DIR}/build/dict)
endif()

if ( NOT ("${HAS_UI}"))
    message("[ai tool] adding ui project")
    add_subdirectory(${CMAKE_SOURCE_DIR}/extern/ui ${CMAKE_SOURCE_DIR}/build/ui)
endif()

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

# AI tool
add_executable (ai-tool "main.c" "Resource.rc") 
add_dependencies(ai-tool json array dict ui)
target_include_directories(ai-tool PUBLIC include ${CMAKE_SOURCE_DIR}/extern/json/include/ ${CMAKE_SOURCE_DIR}/extern/array/include/ ${CMAKE_SOURCE_DIR}/extern/dict/include/ ${CMAKE_SOURCE_DIR}/extern/ui/include/ ) 
target_link_libraries(ai-tool PUBLIC json array dict ui ${SDL2_LIBRARIES} )
