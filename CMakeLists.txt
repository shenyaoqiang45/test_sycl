cmake_minimum_required(VERSION 3.20)

# 直接设置 oneAPI 路径（根据实际安装位置修改）
if(NOT DEFINED ENV{ONEAPI_ROOT})
    set(ONEAPI_ROOT "C:/Program Files (x86)/Intel/oneAPI")
    message(STATUS "ONEAPI_ROOT 环境变量未设置，使用默认路径: ${ONEAPI_ROOT}")
else()
    set(ONEAPI_ROOT "$ENV{ONEAPI_ROOT}")
    message(STATUS "当前 ONEAPI_ROOT 环境变量: '${ONEAPI_ROOT}'")
endif()

# 在 project() 之前设置编译器
if(WIN32 AND EXISTS "${ONEAPI_ROOT}/compiler")
    # 查找最新版本的编译器
    file(GLOB COMPILER_VERSIONS LIST_DIRECTORIES true "${ONEAPI_ROOT}/compiler/*")
    list(SORT COMPILER_VERSIONS)
    list(REVERSE COMPILER_VERSIONS)
    list(GET COMPILER_VERSIONS 0 COMPILER_PATH)
    
    set(CMAKE_C_COMPILER "${COMPILER_PATH}/bin/icx.exe")
    set(CMAKE_CXX_COMPILER "${COMPILER_PATH}/bin/icx.exe")
    
    message(STATUS "设置编译器路径: ${CMAKE_CXX_COMPILER}")
    
    if(EXISTS "${CMAKE_CXX_COMPILER}")
        message(STATUS "✓ 编译器文件存在")
    else()
        message(FATAL_ERROR "✗ 编译器文件不存在: ${CMAKE_CXX_COMPILER}")
    endif()
else()
    message(WARNING "未找到 oneAPI 编译器路径: ${ONEAPI_ROOT}/compiler")
endif()

project(SYCL_Test LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# 检查编译器
if(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
    message(STATUS "✓ 使用 Intel DPC++ 编译器")
    set(USING_DPCPP TRUE)
else()
    message(WARNING "! 未使用 Intel DPC++ 编译器，当前编译器: ${CMAKE_CXX_COMPILER_ID}")
    set(USING_DPCPP FALSE)
endif()

# DPC++/SYCL 编译选项
if(USING_DPCPP)
    # 添加SYCL编译选项
    add_compile_options(/fsycl)
    add_link_options(/fsycl)
    
    # Debug配置
    if(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od /MDd")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG:FULL")
        
        # Release配置
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /MD")
    endif()
endif()

# 添加可执行文件
add_executable(simple_sycl_test simple_sycl_test.cpp)

# 为目标添加SYCL选项
if(USING_DPCPP)
    target_compile_options(simple_sycl_test PRIVATE 
        /fsycl
        $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    )
    target_link_options(simple_sycl_test PRIVATE /fsycl)
endif()

# 设置Visual Studio属性
set_target_properties(simple_sycl_test PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMPILE_PDB_NAME "simple_sycl_test"
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    
    # 设置为启动项目
    VS_STARTUP_PROJECT simple_sycl_test
    
    # 强制使用 DPC++ 平台工具集（关键：使配置属性显示为 DPC++）
    VS_PLATFORM_TOOLSET "Intel(R) oneAPI DPC++ Compiler 2025"
)

# 为 Visual Studio 生成器添加全局属性
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    # 设置为 x64 平台
    if(NOT CMAKE_VS_PLATFORM_NAME)
        set(CMAKE_VS_PLATFORM_NAME "x64" CACHE STRING "Visual Studio Platform" FORCE)
    endif()
endif()

# 输出配置信息
message(STATUS "====================================")
message(STATUS "  SYCL项目配置")
message(STATUS "====================================")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "编译器ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "使用DPC++: ${USING_DPCPP}")
message(STATUS "VS生成器: ${CMAKE_GENERATOR}")
message(STATUS "====================================")