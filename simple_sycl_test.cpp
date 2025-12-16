#include <sycl/sycl.hpp>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>

int main() {
    // 设置控制台为UTF-8编码
    SetConsoleOutputCP(CP_UTF8);
    
    // 图像参数
    const int width = 1280;
    const int height = 960;
    const int total_pixels = width * height;
    const float threshold = 100.0f;
    
    std::cout << "图像分辨率: " << width << " x " << height 
              << " (总像素数: " << total_pixels << ")" << std::endl << std::endl;
    
    // 初始化数据
    std::vector<uint16_t> src_img_0(total_pixels, 100);
    std::vector<uint16_t> src_img_1(total_pixels, 200);
    std::vector<uint16_t> src_img_2(total_pixels, 150);
    std::vector<uint16_t> src_img_3(total_pixels, 180);
    
    std::vector<float> wrapped_phase(total_pixels, 0.0f);
    std::vector<uint8_t> wrapped_mask(total_pixels, 0);
    std::vector<float> mean_intensity(total_pixels, 0.0f);
    
    try {
        // 创建SYCL队列
        sycl::queue queue;
        
        // 打印设备信息
        std::cout << "运行设备: " 
                  << queue.get_device().get_info<sycl::info::device::name>() 
                  << std::endl;
        std::cout << "设备类型: "
                  << (queue.get_device().is_gpu() ? "GPU" : "CPU")
                  << std::endl << std::endl;
        
        // 热身运行
        std::cout << "执行热身运行..." << std::endl;
        {
            sycl::buffer<uint16_t> buf0(src_img_0.data(), sycl::range<1>(total_pixels));
            sycl::buffer<uint16_t> buf1(src_img_1.data(), sycl::range<1>(total_pixels));
            sycl::buffer<uint16_t> buf2(src_img_2.data(), sycl::range<1>(total_pixels));
            sycl::buffer<uint16_t> buf3(src_img_3.data(), sycl::range<1>(total_pixels));
            sycl::buffer<float> phase_buf(wrapped_phase.data(), sycl::range<1>(total_pixels));
            sycl::buffer<uint8_t> mask_buf(wrapped_mask.data(), sycl::range<1>(total_pixels));
            sycl::buffer<float> mean_buf(mean_intensity.data(), sycl::range<1>(total_pixels));
            
            queue.submit([&](sycl::handler& h) {
                auto acc0 = buf0.get_access<sycl::access::mode::read>(h);
                auto acc1 = buf1.get_access<sycl::access::mode::read>(h);
                auto acc2 = buf2.get_access<sycl::access::mode::read>(h);
                auto acc3 = buf3.get_access<sycl::access::mode::read>(h);
                auto phase_acc = phase_buf.get_access<sycl::access::mode::write>(h);
                auto mask_acc = mask_buf.get_access<sycl::access::mode::write>(h);
                auto mean_acc = mean_buf.get_access<sycl::access::mode::write>(h);
                
                h.parallel_for(sycl::range<2>(height, width), [=](sycl::id<2> idx) {
                    const int row = idx[0];
                    const int col = idx[1];
                    const int offset = col + width * row;
                    
                    const float i0 = static_cast<float>(acc0[offset]);
                    const float i1 = static_cast<float>(acc1[offset]);
                    const float i2 = static_cast<float>(acc2[offset]);
                    const float i3 = static_cast<float>(acc3[offset]);
                    
                    const float mean_val = (i0 + i1 + i2 + i3) * 0.25f;
                    mean_acc[offset] = mean_val;
                    
                    const float numerator = i3 - i1;
                    const float denominator = i0 - i2;
                    
                    if (sycl::fabs(numerator) < 1e-6f && sycl::fabs(denominator) < 1e-6f) {
                        phase_acc[offset] = 0.0f;
                        mask_acc[offset] = 0;
                        return;
                    }
                    
                    phase_acc[offset] = sycl::atan2(numerator, denominator);
                    
                    uint8_t mask_val = 1;
                    if (threshold > 0 && mean_val < threshold) {
                        mask_val = 0;
                    }
                    
                    mask_acc[offset] = mask_val;
                });
            });
            
            queue.wait();
        }
        
        std::cout << "✓ 热身运行完成\n" << std::endl;
        
        // 性能测试
        const int num_iterations = 10;
        std::cout << "开始性能测试 (" << num_iterations << " 次迭代)..." << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (int iter = 0; iter < num_iterations; iter++) {
            sycl::buffer<uint16_t> buf0(src_img_0.data(), sycl::range<1>(total_pixels));
            sycl::buffer<uint16_t> buf1(src_img_1.data(), sycl::range<1>(total_pixels));
            sycl::buffer<uint16_t> buf2(src_img_2.data(), sycl::range<1>(total_pixels));
            sycl::buffer<uint16_t> buf3(src_img_3.data(), sycl::range<1>(total_pixels));
            sycl::buffer<float> phase_buf(wrapped_phase.data(), sycl::range<1>(total_pixels));
            sycl::buffer<uint8_t> mask_buf(wrapped_mask.data(), sycl::range<1>(total_pixels));
            sycl::buffer<float> mean_buf(mean_intensity.data(), sycl::range<1>(total_pixels));
            
            queue.submit([&](sycl::handler& h) {
                auto acc0 = buf0.get_access<sycl::access::mode::read>(h);
                auto acc1 = buf1.get_access<sycl::access::mode::read>(h);
                auto acc2 = buf2.get_access<sycl::access::mode::read>(h);
                auto acc3 = buf3.get_access<sycl::access::mode::read>(h);
                auto phase_acc = phase_buf.get_access<sycl::access::mode::write>(h);
                auto mask_acc = mask_buf.get_access<sycl::access::mode::write>(h);
                auto mean_acc = mean_buf.get_access<sycl::access::mode::write>(h);
                
                h.parallel_for(sycl::range<2>(height, width), [=](sycl::id<2> idx) {
                    const int row = idx[0];
                    const int col = idx[1];
                    const int offset = col + width * row;
                    
                    const float i0 = static_cast<float>(acc0[offset]);
                    const float i1 = static_cast<float>(acc1[offset]);
                    const float i2 = static_cast<float>(acc2[offset]);
                    const float i3 = static_cast<float>(acc3[offset]);
                    
                    const float mean_val = (i0 + i1 + i2 + i3) * 0.25f;
                    mean_acc[offset] = mean_val;
                    
                    const float numerator = i3 - i1;
                    const float denominator = i0 - i2;
                    
                    if (sycl::fabs(numerator) < 1e-6f && sycl::fabs(denominator) < 1e-6f) {
                        phase_acc[offset] = 0.0f;
                        mask_acc[offset] = 0;
                        return;
                    }
                    
                    phase_acc[offset] = sycl::atan2(numerator, denominator);
                    
                    uint8_t mask_val = 1;
                    if (threshold > 0 && mean_val < threshold) {
                        mask_val = 0;
                    }
                    
                    mask_acc[offset] = mask_val;
                });
            });
            
            queue.wait();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        // 计算性能指标
        double avg_time_ms = elapsed_ms / num_iterations;
        double fps = 1000.0 / avg_time_ms;
        double gpixels_per_sec = (total_pixels / 1e9) * fps;
        
        std::cout << "\n=== 性能测试结果 ===" << std::endl;
        std::cout << "总耗时: " << elapsed_ms << " ms" << std::endl;
        std::cout << "平均耗时 (单帧): " << avg_time_ms << " ms" << std::endl;
        std::cout << "帧率 (FPS): " << fps << std::endl;
        std::cout << "吞吐量: " << gpixels_per_sec << " GPix/s" << std::endl;
        
        // 验证结果
        std::cout << "\n=== 验证结果 ===" << std::endl;
        std::cout << "样本 [0]: 相位=" << wrapped_phase[0] 
                  << ", 掩码=" << (int)wrapped_mask[0] 
                  << ", 平均强度=" << mean_intensity[0] << std::endl;
        std::cout << "✓ 测试完成!" << std::endl;
        
    } catch (sycl::exception const& e) {
        std::cerr << "SYCL异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}