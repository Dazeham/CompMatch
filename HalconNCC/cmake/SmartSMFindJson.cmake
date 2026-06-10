# Try to find an installed nlohmann_json package (Config mode)
find_package(nlohmann_json QUIET CONFIG)

if(NOT nlohmann_json_FOUND)
    # Use the bundled source if it is not found on the system
    # Assume the json source is located at ${SmartSM_SOURCE_DIR}/external/json
    # Note: CMAKE_CURRENT_LIST_DIR points to the cmake/ directory, so go up two levels
    set(JSON_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../dependence/json")
    if(EXISTS "${JSON_ROOT_DIR}/CMakeLists.txt")
        message(STATUS "nlohmann_json not found in system, using built-in version from ${JSON_ROOT_DIR}")
        # Optional: disable json test builds
        set(JSON_BuildTests OFF CACHE INTERNAL "")
        add_subdirectory(${JSON_ROOT_DIR} ${CMAKE_CURRENT_LIST_DIR}/../dependence/json)
    else()
        message(FATAL_ERROR "nlohmann_json not found and built-in source missing at ${JSON_ROOT_DIR}")
    endif()
endif()