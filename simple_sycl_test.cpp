#include <sycl/sycl.hpp>
#include <iostream>
#include <vector>
#include <Windows.h>

int main() {
    // 设置控制台为UTF-8编码
    SetConsoleOutputCP(CP_UTF8);
    
    // 数组大小
    const size_t N = 1024;
    
    // 初始化数据
    std::vector<float> a(N, 1.0f);
    std::vector<float> b(N, 2.0f);
    std::vector<float> c(N, 0.0f);
    
    try {
        // 创建SYCL队列
        sycl::queue q;
        
        // 打印设备信息
        std::cout << "运行设备: " 
                  << q.get_device().get_info<sycl::info::device::name>() 
                  << std::endl;
        
        {
            // 创建缓冲区
            sycl::buffer<float> buf_a(a.data(), sycl::range<1>(N));
            sycl::buffer<float> buf_b(b.data(), sycl::range<1>(N));
            sycl::buffer<float> buf_c(c.data(), sycl::range<1>(N));
            
            // 提交内核
            q.submit([&](sycl::handler& h) {
                // 获取访问器
                auto acc_a = buf_a.get_access<sycl::access::mode::read>(h);
                auto acc_b = buf_b.get_access<sycl::access::mode::read>(h);
                auto acc_c = buf_c.get_access<sycl::access::mode::write>(h);
                
                // 定义并行内核
                h.parallel_for(sycl::range<1>(N), [=](sycl::id<1> idx) {
                    acc_c[idx] = acc_a[idx] + acc_b[idx];
                });
            });
            
            // 等待计算完成
            q.wait();
        } // buffer析构，自动复制数据回主机
        
        // 验证结果
        bool success = true;
        for (size_t i = 0; i < N; i++) {
            if (c[i] != 3.0f) {
                success = false;
                std::cout << "错误: c[" << i << "] = " << c[i] << std::endl;
                break;
            }
        }
        
        if (success) {
            std::cout << "✓ 测试通过! 向量加法计算正确。" << std::endl;
            std::cout << "示例结果: " << a[0] << " + " << b[0] 
                      << " = " << c[0] << std::endl;
        }
        
    } catch (sycl::exception const& e) {
        std::cerr << "SYCL异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}