#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_message.H>  // 用于FLTK弹窗提示
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
 
 
#include <chrono>
#include <string>
#include <vector>

// 应用程序核心数据结构
struct CameraApp {
    cv::VideoCapture cap;               // 摄像头捕获对象
    cv::Mat frame;                      // 原始帧缓存
    cv::Mat frame_rgb;                  // RGB格式帧（预分配）
    cv::Mat cached_frame;               // 双缓冲缓存帧（预分配）
    Fl_RGB_Image* fltk_img = nullptr;   // FLTK图像对象
    Fl_Box* display_box = nullptr;      // 显示区域
    Fl_Window* window = nullptr;        // 主窗口
    bool is_running = true;             // 运行状态
    int optimal_width = 640;            // 最佳宽度（自动检测）
    int optimal_height = 480;           // 最佳高度（自动检测）
    double optimal_fps = 30;            // 最佳帧率（自动检测）
} app;

// 自动检测摄像头支持的分辨率（从高到低探测）
void detect_resolutions() {
    const std::vector<std::pair<int, int>> resolutions = {
        {3840, 2160}, {2560, 1440}, {1920, 1080},
        {1280, 720}, {1280, 960}, {800, 600}, {640, 480}
    };

    for (const auto& res : resolutions) {
        app.cap.set(cv::CAP_PROP_FRAME_WIDTH, res.first);
        app.cap.set(cv::CAP_PROP_FRAME_HEIGHT, res.second);

        int actual_w = static_cast<int>(app.cap.get(cv::CAP_PROP_FRAME_WIDTH));
        int actual_h = static_cast<int>(app.cap.get(cv::CAP_PROP_FRAME_HEIGHT));

        if (actual_w == res.first && actual_h == res.second) {
            app.optimal_width = actual_w;
            app.optimal_height = actual_h;
            return;
        }
    }
}

// 自动检测摄像头支持的帧率（从高到低探测）
void detect_fps() {
    const std::vector<double> fps_candidates = { 60, 50, 30, 25, 24, 15, 10 };

    for (double fps : fps_candidates) {
        app.cap.set(cv::CAP_PROP_FPS, fps);
        double actual_fps = app.cap.get(cv::CAP_PROP_FPS);

        if (std::abs(actual_fps - fps) < 1.0) {
            app.optimal_fps = actual_fps;
            return;
        }
    }

    app.optimal_fps = app.cap.get(cv::CAP_PROP_FPS);
}

// 截图功能（使用FLTK弹窗提示结果）
void screenshot_callback(Fl_Widget* widget, void* data) {
    if (app.cached_frame.empty()) {
        fl_alert("截图失败", "没有可保存的图像数据");  // 弹窗提示错误
        return;
    }

    // 生成带时间戳的文件名
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::string filename = "screenshot_" + std::to_string(now_time) + ".png";

    // 转换回BGR格式保存
    cv::Mat bgr_frame;
    cv::cvtColor(app.cached_frame, bgr_frame, cv::COLOR_RGB2BGR);

    if (cv::imwrite(filename, bgr_frame)) {
        fl_message_title("截图成功");                  // 弹窗标题
        fl_message("图像已保存为:\n%s", filename.c_str());  // 弹窗内容
    }
    else {
        fl_alert("截图失败", "无法保存图像文件");      // 错误弹窗
    }
}

// 视频帧更新回调
void update_frame(void* data) {
    if (!app.is_running) return;

    // 异步捕获帧（4.12.0队列优化）
    if (!app.cap.grab()) {
        fl_alert("摄像头错误", "无法捕获视频帧，程序将退出");
        app.is_running = false;
        return;
    }

    // 解码获取帧数据
    if (!app.cap.retrieve(app.frame)) {
        fl_alert("摄像头错误", "无法解码视频帧，程序将退出");
        app.is_running = false;
        return;
    }


   
        // 如果没有GPU，则使用CPU处理
        cv::resize(app.frame, app.frame, cv::Size(app.optimal_width, app.optimal_height));
        cv::cvtColor(app.frame, app.frame_rgb, cv::COLOR_BGR2RGB);
 
    // 双缓冲复制（4.12.0连续内存优化）
    app.frame_rgb.copyTo(app.cached_frame);

    // 更新FLTK图像（4.12.0内存指针优化）
    if (app.fltk_img) {
        app.display_box->image(nullptr);
        delete app.fltk_img;
    }
    app.fltk_img = new Fl_RGB_Image(
        app.cached_frame.ptr(),
        app.cached_frame.cols,
        app.cached_frame.rows,
        3,
        0
    );

    app.display_box->image(app.fltk_img);
    app.display_box->redraw();
    app.window->redraw();
    Fl::flush();

    // 按最佳帧率设置下一次更新
    Fl::repeat_timeout(1.0 / app.optimal_fps, update_frame);
}

// 窗口关闭回调
void window_close_callback(Fl_Widget* widget, void* data) {
    app.is_running = false;
    app.cap.release();
    if (app.fltk_img) {
        app.display_box->image(nullptr);
        delete app.fltk_img;
    }
    app.window->hide();
}

int main(int argc, char* argv[]) {
    // 初始化摄像头
    app.cap.open(0, cv::CAP_ANY);
    if (!app.cap.isOpened()) {
        fl_alert("初始化失败", "无法打开摄像头，请检查设备是否连接");
        return -1;
    }

    // 自动检测最佳参数
    detect_resolutions();
    detect_fps();

    // 预分配内存
    app.frame_rgb.create(app.optimal_height, app.optimal_width, CV_8UC3);
    app.cached_frame.create(app.optimal_height, app.optimal_width, CV_8UC3);

    // 启用自动参数
    app.cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
    app.cap.set(cv::CAP_PROP_AUTOFOCUS, 1);

    // 创建FLTK窗口
    app.window = new Fl_Window(
        app.optimal_width,
        app.optimal_height + 30,
        "摄像头监控（FLTK界面）"
    );
    app.window->callback(window_close_callback);

    // 创建显示区域和按钮
    app.display_box = new Fl_Box(0, 0, app.optimal_width, app.optimal_height);
    app.display_box->box(FL_FLAT_BOX);

    Fl_Button* screenshot_btn = new Fl_Button(
        (app.optimal_width - 100) / 2,
        app.optimal_height + 5,
        100, 25,
        "截图"
    );
    screenshot_btn->callback(screenshot_callback);

    app.window->end();
    app.window->show(argc, argv);

    // 启动帧更新循环
    Fl::add_timeout(0.0, update_frame);

    return Fl::run();
}