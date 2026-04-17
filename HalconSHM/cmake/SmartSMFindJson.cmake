# 尝试查找系统中已安装的 nlohmann_json（Config 模式）
find_package(nlohmann_json QUIET CONFIG)

if(NOT nlohmann_json_FOUND)
    # 如果系统中未找到，则使用项目内自带的源码
    # 假设 json 源码位于 ${SmartSM_SOURCE_DIR}/external/json
    # 注意：CMAKE_CURRENT_LIST_DIR 指向 cmake/ 目录，所以需要上两级
    set(JSON_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../dependence/json")
    if(EXISTS "${JSON_ROOT_DIR}/CMakeLists.txt")
        message(STATUS "nlohmann_json not found in system, using built-in version from ${JSON_ROOT_DIR}")
        # 可选：关闭 json 自带的测试构建
        set(JSON_BuildTests OFF CACHE INTERNAL "")
        add_subdirectory(${JSON_ROOT_DIR} ${CMAKE_CURRENT_LIST_DIR}/../dependence/json)
    else()
        message(FATAL_ERROR "nlohmann_json not found and built-in source missing at ${JSON_ROOT_DIR}")
    endif()
endif()