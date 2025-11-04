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

# ==================== 1. FLTK 自动构建（新增 ExternalProject）====================
# 注意：我们使用 fltk-1.4.x 的 Git 仓库
ExternalProject_Add(
    fltk_AutoBuild # <-- 新增 FLTK 外部项目名称
    
    EXCLUDE_FROM_ALL ON
    GIT_REPOSITORY https://github.com/fltk/fltk.git
    GIT_TAG release-1.3.5 # <-- 确保使用你需要的版本标签
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
        -DFLTK_USE_SYSTEM_LIBPNG=OFF # 确保使用其内置的 PNG 库
        -DFLTK_USE_SYSTEM_LIBJPEG=OFF # 确保使用其内置的 JPEG 库
        -DOPTION_USE_THREADS=ON # 启用多线程支持
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

# 区分 Debug/Release 库名 <-- 移除/注释掉这部分
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(FLTK_CORE_LIB_NAME "fltkd")
# else()
#     set(FLTK_CORE_LIB_NAME "fltk")
# endif()
set(FLTK_CORE_LIB_NAME "fltk") # <-- 统一使用 fltk


# ==================== 2. OpenCV 自动构建（调整参数）====================
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
    
    # OpenCV 编译选项（只保留必需模块，禁用所有可能冲突的选项）
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        
        # 核心：禁用所有复杂的特性，以确保配置阶段通过
        -DWITH_CUDA=OFF              # <-- 禁用 CUDA
        -DWITH_FFMPEG=OFF            # <-- 禁用 FFMPEG (避免下载冲突)
        -DBUILD_opencv_freetype=OFF  # <-- 禁用 FREETYPE/中文支持 (避免 Contrib 冲突)
        -DBUILD_opencv_text=OFF      # <-- 禁用 TEXT
        -DWITH_IPP=OFF               # 禁用 IPP
        -DWITH_OPENCL=OFF            # 禁用 OpenCL
        -DWITH_LAPACK=OFF            # 禁用 LAPACK
        -DBUILD_SHARED_LIBS=ON       # 编译动态库
        # 核心修正：禁用 calib3d 和 ml
        -DBUILD_opencv_calib3d=OFF
        -DBUILD_opencv_ml=OFF
        
        # 最终修正：禁用 DNN 模块（排除 Protobuf/Flatbuffers 等复杂依赖链）
        -DBUILD_opencv_dnn=OFF # <--- 强烈建议新增！
        -DBUILD_opencv_core=ON       # 核心
        -DBUILD_opencv_imgproc=ON    # 图像处理
        -DBUILD_opencv_videoio=ON    # 视频/摄像头
        -DBUILD_opencv_imgcodecs=ON  # 图像编解码
        
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_STANDARD=17
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTS=OFF
		# 禁用 Python 模块，消除 numpy 依赖报错
        -DBUILD_opencv_python_bindings=OFF
        -DBUILD_opencv_python_tests=OFF
        
        # 统一运行时库
        -DCMAKE_C_FLAGS_RELEASE="/MD"
        -DCMAKE_C_FLAGS_DEBUG="/MDd"
        -DCMAKE_CXX_FLAGS_RELEASE="/MD"
        -DCMAKE_CXX_FLAGS_DEBUG="/MDd"

)

# 配置 OpenCV 路径
set(OPENCV_INSTALL_DIR ${CMAKE_BINARY_DIR}/External/OpenCV/Install)
set(OPENCV_INCLUDE_DIR ${OPENCV_INSTALL_DIR}/include)
set(OPENCV_LIB_DIR ${OPENCV_INSTALL_DIR}/x64/vc16/lib)

# 区分 Debug/Release 库名（保持你原来的逻辑）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OPENCV_LIB_SUFFIX "d")
else()
    set(OPENCV_LIB_SUFFIX "")
endif()

set(OPENCV_LIBS
    ${OPENCV_LIB_DIR}/opencv_core4120${OPENCV_LIB_SUFFIX}.lib
    ${OPENCV_LIB_DIR}/opencv_imgproc4120${OPENCV_LIB_SUFFIX}.lib
    ${OPENCV_LIB_DIR}/opencv_videoio4120${OPENCV_LIB_SUFFIX}.lib
    ${OPENCV_LIB_DIR}/opencv_imgcodecs4120${OPENCV_LIB_SUFFIX}.lib
)

# ==================== 3. 主项目编译与链接 ====================
include_directories(${FLTK_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR})
link_directories(${FLTK_LIB_DIR}) # FLTK 库的路径
link_directories(${OPENCV_LIB_DIR}) # OpenCV 库的路径


add_executable(fltk_chinese_test main.cpp)

# 确保 FLTK 和 OpenCV 都编译安装完成后，再编译主项目
add_dependencies(fltk_chinese_test fltk_AutoBuild fltk_cv_AutoBuild) # <-- 新增 FLTK 依赖

# 链接所有库
target_link_libraries(fltk_chinese_test
    ${FLTK_CORE_LIB_NAME} # FLTK 核心库
	fltk_images
    ${OPENCV_LIBS}
    gdiplus
    ws2_32
    user32
    comctl32
)