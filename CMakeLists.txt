cmake_minimum_required(VERSION 3.10)
project(fltk_chinese_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)

# 确保所有目标都使用动态链接的运行时库（消除兼容性问题）
if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE "/MD")
    set(CMAKE_C_FLAGS_DEBUG "/MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd")
    add_compile_options("/utf-8") # 支持中文
endif()

# ==================== 1. FLTK 自动构建 ====================
ExternalProject_Add(
    fltk_AutoBuild 
    
    EXCLUDE_FROM_ALL ON
    GIT_REPOSITORY https://github.com/fltk/fltk.git
    GIT_TAG release-1.3.5
    GIT_SHALLOW TRUE

    PREFIX ${CMAKE_BINARY_DIR}/External/FLTK
    BINARY_DIR ${CMAKE_BINARY_DIR}/External/FLTK/build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/External/FLTK/Install

    # 关键：指定 FLTK 编译的生成器和架构，匹配 Runner 环境
    CMAKE_GENERATOR "Visual Studio 17 2022"
    CMAKE_GENERATOR_PLATFORM x64
    CMAKE_GENERATOR_TOOLSET "v143"

    # FLTK 编译选项
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DOPTION_BUILD_EXAMPLES=OFF
        -DOPTION_BUILD_TEST=OFF
		-DFLTK_BUILD_HTML_DOCS=OFF
        -DFLTK_USE_SYSTEM_LIBPNG=OFF
        -DFLTK_USE_SYSTEM_LIBJPEG=OFF
        -DOPTION_USE_THREADS=ON
        # 统一运行时库
        -DCMAKE_C_FLAGS_RELEASE="/MD"
        -DCMAKE_C_FLAGS_DEBUG="/MDd"
        -DCMAKE_CXX_FLAGS_RELEASE="/MD"
        -DCMAKE_CXX_FLAGS_DEBUG="/MDd"
		
)

# 配置 FLTK 路径
set(FLTK_INSTALL_DIR ${CMAKE_BINARY_DIR}/External/FLTK/Install)
set(FLTK_INCLUDE_DIR ${FLTK_INSTALL_DIR}/include)
set(FLTK_LIB_DIR ${FLTK_INSTALL_DIR}/lib)

# 移除手动设置后缀的代码。

# ==================== 2. OpenCV 自动构建 ====================
ExternalProject_Add(
    fltk_cv_AutoBuild
    
    EXCLUDE_FROM_ALL ON
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG 4.12.0
    GIT_SHALLOW TRUE
    
    PREFIX ${CMAKE_BINARY_DIR}/External/OpenCV
    BINARY_DIR ${CMAKE_BINARY_DIR}/External/OpenCV/build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/External/OpenCV/Install
    
    # 关键：VS 2022 配置
    CMAKE_GENERATOR "Visual Studio 17 2022"
    CMAKE_GENERATOR_PLATFORM x64
    CMAKE_GENERATOR_TOOLSET "v143"
    
    # OpenCV 编译选项（保持不变，它工作正常）
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DWITH_CUDA=OFF
        -DWITH_FFMPEG=OFF
        -DBUILD_opencv_freetype=OFF
        -DBUILD_opencv_text=OFF
        -DWITH_IPP=OFF
        -DWITH_OPENCL=OFF
        -DWITH_LAPACK=OFF
        -DBUILD_SHARED_LIBS=ON
        -DBUILD_opencv_calib3d=OFF
        -DBUILD_opencv_ml=OFF
        -DBUILD_JPEG=OFF
        -DWITH_JPEG=OFF
        -DOPENCV_ENABLE_NASM=OFF
        -DENABLE_SSE=OFF
        -DBUILD_opencv_dnn=OFF
        -DBUILD_opencv_core=ON
        -DBUILD_opencv_imgproc=ON
        -DBUILD_opencv_videoio=ON
        -DBUILD_opencv_imgcodecs=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_STANDARD=17
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTS=OFF
        -DBUILD_opencv_python_bindings=OFF
        -DBUILD_opencv_python_tests=OFF
        -DOPENCV_ENABLE_NASM=OFF
        -DENABLE_SSE=OFF
        -DBUILD_JAVA=OFF
        -DCMAKE_C_FLAGS_RELEASE="/MD"
        -DCMAKE_C_FLAGS_DEBUG="/MDd"
        -DCMAKE_CXX_FLAGS_RELEASE="/MD"
        -DCMAKE_CXX_FLAGS_DEBUG="/MDd"
)

# 配置 OpenCV 路径
set(OPENCV_INSTALL_DIR ${CMAKE_BINARY_DIR}/External/OpenCV/Install)
set(OPENCV_INCLUDE_DIR ${OPENCV_INSTALL_DIR}/include)
# 修正路径：使用生成器表达式来处理 vc17 目录和 Debug/Release 库名

# <<< 关键修改开始 >>>
# 定义一个宏，用于构造 Windows 平台下带后缀的库名
# $<CONFIG:Debug> 返回 Debug 配置名，否则为空
# $<IF> 用于判断是否是 Debug 配置，如果是则添加 'd' 后缀

# 构造动态的库目录（基于 Visual Studio 版本）
set(OPENCV_LIB_DIR "${OPENCV_INSTALL_DIR}/x64/$<IF:$<STREQUAL:${CMAKE_GENERATOR_TOOLSET},v143>,vc17,vc16>/lib")
# 简单粗暴点，直接使用 vc17 目录，因为你已经用 VS 2022 编译了：
set(OPENCV_LIB_DIR_VC17 "${OPENCV_INSTALL_DIR}/x64/vc17/lib")


# 构造 FLTK 库名：fltk.lib 或 fltkd.lib
set(FLTK_LIBS
    "${FLTK_LIB_DIR}/fltk$<IF:$<CONFIG:Debug>,d,.>.lib"
    "${FLTK_LIB_DIR}/fltk_images$<IF:$<CONFIG:Debug>,d,.>.lib"
)

# 构造 OpenCV 库名：opencv_core4120.lib 或 opencv_core4120d.lib
set(OPENCV_LIBS
    "${OPENCV_LIB_DIR_VC17}/opencv_core4120$<IF:$<CONFIG:Debug>,d,.>.lib"
    "${OPENCV_LIB_DIR_VC17}/opencv_imgproc4120$<IF:$<CONFIG:Debug>,d,.>.lib"
    "${OPENCV_LIB_DIR_VC17}/opencv_videoio4120$<IF:$<CONFIG:Debug>,d,.>.lib"
    "${OPENCV_LIB_DIR_VC17}/opencv_imgcodecs4120$<IF:$<CONFIG:Debug>,d,.>.lib"
)
# <<< 关键修改结束 >>>


# ==================== 3. 主项目编译与链接 ====================
include_directories(${FLTK_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR})
# 移除 link_directories，因为我们使用的是完整的库路径，无需 link_directories


add_executable(fltk_chinese_test main.cpp)

# 确保 FLTK 和 OpenCV 都编译安装完成后，再编译主项目
add_dependencies(fltk_chinese_test fltk_AutoBuild fltk_cv_AutoBuild)

# 链接所有库
target_link_libraries(fltk_chinese_test
    PRIVATE
        ${FLTK_LIBS} # FLTK 核心库和图像库
        ${OPENCV_LIBS} # OpenCV 核心功能库
        gdiplus
        ws2_32
        user32
        comctl32
)