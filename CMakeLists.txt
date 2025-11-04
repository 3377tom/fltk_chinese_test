cmake_minimum_required(VERSION 3.10)
project(fltk_chinese_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== FLTK 配置（不变，使用本地路径）====================
set(FLTK_ROOT "E:/code/fltk-1.4.4")
set(FLTK_INCLUDE_DIR "${FLTK_ROOT}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FLTK_LIB_DIR "${FLTK_ROOT}/build/lib/Debug")
    set(FLTK_CORE_LIB "fltkd")
else()
    set(FLTK_LIB_DIR "${FLTK_ROOT}/build/lib/Release")
    set(FLTK_CORE_LIB "fltk")
endif()

include_directories(${FLTK_INCLUDE_DIR})
link_directories(${FLTK_LIB_DIR})

# ==================== OpenCV 自动构建（彻底独立配置，避免参数冲突）====================
include(ExternalProject)

# 定义 OpenCV 编译的独立参数，不继承主项目的生成器参数
ExternalProject_Add(
    fltk_cv_AutoBuild
     
    EXCLUDE_FROM_ALL ON 
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG 4.12.0
    GIT_SHALLOW TRUE  # 只下载最新提交，加速下载
    
    # 独立的构建、安装目录（彻底隔离主项目）
    PREFIX ${CMAKE_BINARY_DIR}/External/OpenCV
    BINARY_DIR ${CMAKE_BINARY_DIR}/External/OpenCV/build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/External/OpenCV/Install
    
    # 关键：显式指定 OpenCV 编译的生成器和架构，不依赖主项目传递
    # 将 VS 16 2019 替换为 VS 17 2022
    CMAKE_GENERATOR "Visual Studio 17 2022" 
    CMAKE_GENERATOR_PLATFORM x64 
    CMAKE_GENERATOR_TOOLSET "v143"  # VS 2022 工具集是 v143
    
    # OpenCV 编译选项（只保留必需模块，加速编译）
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>  # 安装目录
         
               -DWITH_FFMPEG=ON
        -DOPENCV_DOWNLOAD_FFMPEG=OFF
        -DOPENCV_FFMPEG_SKIP_DOWNLOAD=ON
        -DOPENCV_FFMPEG_BINARIES_PATH="E:/opencv_ffmpeg"
        -DOPENCV_DOWNLOAD_EXTERNAL=OFF
        -DCMAKE_USE_OPENSSL=ON
        -DBUILD_opencv_videoio=ON  # 摄像头模块（必需）
        -DBUILD_opencv_imgcodecs=ON  # 截图保存模块（必需）
        -DBUILD_opencv_core=ON  # 核心模块
        -DBUILD_opencv_imgproc=ON  # 图像处理模块
        -DBUILD_SHARED_LIBS=ON  # 编译动态库
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}  # 同步 Debug/Release
        -DCMAKE_CXX_STANDARD=17  # 匹配 C++ 标准
        -DWITH_WIN32UI=ON  # 兼容 Windows UI
        -DBUILD_EXAMPLES=OFF  # 禁用示例（加速）
        -DBUILD_TESTS=OFF  # 禁用测试（加速）
        -DBUILD_DOCS=OFF  # 禁用文档（加速）
        -DWITH_IPP=OFF  # 禁用 IPP（减少依赖冲突）
        -DWITH_OPENCL=OFF  # 禁用 OpenCL（简化配置）
   -DBUILD_opencv_cudev=ON  # CUDA设备层基础模块（必需）
    -DWITH_CUBLAS=ON         # 启用CUDA BLAS加速（矩阵运算优化）
        -D WITH_CUDA=ON
         -DBUILD_opencv_cudaarithm=ON  # 启用 CUDA 算术库
        -DBUILD_opencv_cudaimgproc=ON  # 启用 CUDA 图像处理库
        -DBUILD_opencv_cudawarping=ON  
 -DBUILD_opencv_cudawarping=OFF  # 若不需要透视变换等功能
       -D ENABLE_FAST_MATH=ON
      -D CUDA_FAST_MATH=ON
     -DOPENCV_EXTRA_MODULES_PATH=E:/opencv_contrib/modules
        -DCUDA_ARCH_BIN=ALL # 替换为你的显卡计算能力（如RTX30系为8.6）
 
   # 禁用LAPACK/BLAS依赖（核心解决参数）
        -DWITH_LAPACK=OFF
        -DWITH_BLAS=OFF
        # 同时禁用依赖LAPACK的可选模块（进一步避免冲突）
        -DBUILD_opencv_calib3d=OFF  # 相机标定模块（依赖LAPACK）
        -DBUILD_opencv_ml=OFF       # 机器学习模块（依赖LAPACK）
        # 禁用依赖calib3d的contrib模块（核心解决当前错误）
        -DBUILD_opencv_stereo=OFF  # 立体匹配模块（直接依赖calib3d）
        -DBUILD_opencv_surface_matching=OFF  # 表面匹配模块（依赖calib3d）
        -DBUILD_opencv_xphoto=OFF  # 扩展摄影模块（可能间接依赖）
        # 禁用其他非必需的contrib模块（进一步避免冲突）
        -DBUILD_opencv_xfeatures2d=OFF
        -DBUILD_opencv_ximgproc=OFF
        -DBUILD_opencv_bgsegm=OFF
            # 核心：启用 freetype 模块（中文支持）
    -DBUILD_opencv_freetype=ON
    # 自动下载 freetype 库（核心依赖）
    -DOPENCV_DOWNLOAD_FREETYPE=ON
    # 自动下载 bz2 库（支持 .bz2 压缩字体）
    -DOPENCV_DOWNLOAD_BZIP2=ON
    # 自动下载 brotli 库（支持 Brotli 压缩字体）
    -DOPENCV_DOWNLOAD_BROTLI=ON
    # 自动下载 harfbuzz 库（复杂中文排版）
    -DOPENCV_DOWNLOAD_HARFBUZZ=ON
    # 统一运行时库（解决之前的LNK4098警告）
    -DCMAKE_CXX_FLAGS_RELEASE="/MD"
    -DCMAKE_CXX_FLAGS_DEBUG="/MDd"
    -DCMAKE_C_FLAGS_RELEASE="/MD"
    -DCMAKE_C_FLAGS_DEBUG="/MDd"
-DBUILD_WITH_STATIC_CRT=OFF
       # 修正CUDA编译选项（核心）
        -DCUDA_NVCC_FLAGS=--compiler-options=/MD
        -DCUDA_NVCC_FLAGS_DEBUG=--compiler-options=/MDd
)

# 配置 OpenCV 路径（对应上面的 INSTALL_DIR）
set(OPENCV_INSTALL_DIR ${CMAKE_BINARY_DIR}/External/OpenCV/Install)
set(OPENCV_INCLUDE_DIR ${OPENCV_INSTALL_DIR}/include)

# 区分 Debug/Release 库（根据 OpenCV 编译结果）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OPENCV_LIB_DIR ${OPENCV_INSTALL_DIR}/x64/vc16/lib)
        # Debug 模式：库名带 d 后缀
     set(OPENCV_LIBS
    # CPU核心库（必须）
    ${OPENCV_LIB_DIR}/opencv_core4120d.lib
    ${OPENCV_LIB_DIR}/opencv_imgproc4120d.lib
    ${OPENCV_LIB_DIR}/opencv_videoio4120d.lib
    ${OPENCV_LIB_DIR}/opencv_imgcodecs4120d.lib
    # CUDA基础库（新增）
    ${OPENCV_LIB_DIR}/opencv_cudev4120d.lib
    # CUDA算术与图像处理库
    ${OPENCV_LIB_DIR}/opencv_cudaarithm4120d.lib
    ${OPENCV_LIB_DIR}/opencv_cudaimgproc4120d.lib
    ${OPENCV_LIB_DIR}/opencv_cudawarping4120d.lib
    # CUDA BLAS库（新增，若启用WITH_CUBLAS）
    ${OPENCV_LIB_DIR}/opencv_cublas4120d.lib
    )
else()
    set(OPENCV_LIB_DIR ${OPENCV_INSTALL_DIR}/x64/vc16/lib)
 set(OPENCV_LIBS
          # CPU核心库（必须）
        ${OPENCV_LIB_DIR}/opencv_core4120.lib
        ${OPENCV_LIB_DIR}/opencv_imgproc4120.lib
        ${OPENCV_LIB_DIR}/opencv_videoio4120.lib
        ${OPENCV_LIB_DIR}/opencv_imgcodecs4120.lib
       # CUDA基础库（新增）
        ${OPENCV_LIB_DIR}/opencv_cudev4120.lib
        # CUDA算术与图像处理库
        ${OPENCV_LIB_DIR}/opencv_cudaarithm4120.lib
        ${OPENCV_LIB_DIR}/opencv_cudaimgproc4120.lib
        ${OPENCV_LIB_DIR}/opencv_cudawarping4120.lib
        # CUDA BLAS库（新增，若启用WITH_CUBLAS）
        ${OPENCV_LIB_DIR}/opencv_cublas4120.lib
    )
endif()

include_directories(${OPENCV_INCLUDE_DIR})
link_directories(${OPENCV_LIB_DIR})

# ==================== 编译器配置（MSVC 专属）====================
if(MSVC)
    add_compile_options("/utf-8")  # 支持中文
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif()

# ==================== 编译与链接 ====================
add_executable(fltk_chinese_test 
    main.cpp 
     
)

# 确保 OpenCV 编译完成后，再编译主项目
add_dependencies(fltk_chinese_test fltk_cv_AutoBuild)

# 链接所有库
target_link_libraries(fltk_chinese_test 
    ${FLTK_CORE_LIB}
     ${OPENCV_LIBS} 
    gdiplus
    ws2_32
    user32
    comctl32
)