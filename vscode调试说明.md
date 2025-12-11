# VSCode 调试 SYCL 项目使用指南

## 📋 环境要求

### 必需软件
1. **Visual Studio Code**
2. **VSCode 扩展**
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft) - 可选
3. **Intel oneAPI Toolkit**
   - 默认路径：`C:\Program Files (x86)\Intel\oneAPI`
4. **CMake** (≥ 3.20)
5. **Ninja** 构建工具

---

## 📁 项目结构

```
f:\test_sycl\
├── .vscode/
│   ├── launch.json          # 调试配置
│   └── tasks.json           # 构建任务
├── build/                   # 构建输出目录（Ninja）
│   └── simple_sycl_test.exe # 可执行文件
├── CMakeLists.txt
├── simple_sycl_test.cpp
└── vscode调试说明.md
│   └── tasks.json                # 构建任务配置
├── build-vs/                     # CMake 构建输出目录
│   └── Debug/
│       └── simple_sycl_test.exe  # 可执行文件
├── CMakeLists.txt                # CMake 项目配置
├── simple_sycl_test.cpp          # SYCL 源代码
└── vscode调试说明.md             # 本文档
```

---

## ⚙️ 配置文件说明

### 1. `.vscode/settings.json` - 工作区设置

```json
{
    "cmake.generator": "Ninja",                    // 使用 Ninja 构建
    "cmake.configureOnOpen": false,                // 打开时不自动配置
    "cmake.buildDirectory": "${workspaceFolder}/build-vs",
    "cmake.configureSettings": {
        "CMAKE_BUILD_TYPE": "Debug"                // Debug 模式
```

**注意**：使用 Ninja 生成器时，可执行文件直接在 `build/` 根目录，不在 `Debug/` 子目录。

---

## ⚙️ 配置文件说明

### 1. `.vscode/launch.json` - 调试配置

```jsonc
{
    "name": "Debug SYCL Test",
    "type": "cppvsdbg",              // Visual Studio 调试器
    "program": "${workspaceFolder}/build/simple_sycl_test.exe",
    "console": "integratedTerminal", // 使用集成终端
    "preLaunchTask": "CMake: Build"  // 调试前自动构建
}
```

**关键点**：
- `type: cppvsdbg` - 使用 Windows Visual Studio 调试器引擎
- 可执行文件路径：`build/simple_sycl_test.exe`（Ninja 特性）
- 自动构建：按 F5 会先执行构建任务

### 2. `.vscode/tasks.json` - 构建任务

| 任务名称 | 功能 | 说明 |
|---------|------|------|
| **CMake: Configure** | 配置 CMake 项目 | 生成 Ninja 构建文件 |
| **CMake: Build** | 构建项目 | Debug 模式，默认任务 |
| **CMake: Clean** | 清理构建 | 删除 build 目录 |
| **CMake: Rebuild** | 重新构建 | 先清理再构建 |

**特点**：
- 使用 **cmd.exe** shell
- 每个任务都调用 `setvars.bat` 初始化 Intel 环境
- 使用 **Ninja** 生成器
- 构建模式：**Debug**（包含调试符号）
- 编译器：**Intel icx**

### 3. `.vscode/c_cpp_properties.json` - IntelliSense 配置

**可选文件**。如果有 `"configurationProvider": "ms-vscode.cmake-tools"`，可以删除此文件，CMake Tools 会自动提供 IntelliSense。

如果 IntelliSense 有问题，保留此配置：
```jsonc
{
    "compilerPath": "C:/Program Files (x86)/Intel/oneAPI/compiler/latest/bin/icx.exe",
    "cppStandard": "c++17",
    "intelliSenseMode": "windows-clang-x64"  // 注意：windows 不是 linux
}
```

---

## 🚀 快速开始

### 第一次使用

1. **打开项目**
   ```
   文件 → 打开文件夹 → 选择 f:\test_sycl
   ```

2. **开始调试**
   - 打开 `simple_sycl_test.cpp`
   - 在代码行号左侧点击设置断点（红点）
   - 按 **F5** 启动调试

3. **自动流程**
   - ✓ 调用 setvars.bat 初始化环境
   - ✓ CMake 配置项目（Ninja + icx + Debug）
   - ✓ 编译代码
   - ✓ 启动调试器并停在断点

### 日常使用

| 操作 | 快捷键 / 方法 |
|------|--------------|
| **开始调试** | `F5` |
| **快速构建** | `Ctrl+Shift+B` |
| **设置/取消断点** | `F9` 或点击行号 |
| **单步跳过** | `F10` |
| **单步进入** | `F11` |
| **单步跳出** | `Shift+F11` |
| **停止调试** | `Shift+F5` |
| **打开任务列表** | `Ctrl+Shift+P` → `Tasks: Run Task` |

---

## 🔧 手动构建（可选）

如果不想使用 F5 自动构建，可以手动执行：

1. **打开命令面板**：`Ctrl+Shift+P`

2. **选择任务**：`Tasks: Run Task`

3. **可用任务**：
   - `CMake: Configure` - 配置项目
   - `CMake: Build` - 构建项目（或按 `Ctrl+Shift+B`）
   - `CMake: Clean` - 清理
   - `CMake: Rebuild` - 重新构建

---

## 🐛 调试功能

### 调试面板（左侧）

1. **变量（Variables）**
   - 查看局部变量、参数
   - 展开对象查看成员

2. **监视（Watch）**
   - 点击 `+` 添加自定义表达式
   - 例如：`c[0]`, `a.size()`, `q.get_device()`

3. **调用堆栈（Call Stack）**
   - 查看函数调用链
   - 点击可跳转

4. **断点（Breakpoints）**
   - 管理所有断点
   - 可启用/禁用

### 高级断点

**条件断点**：
- 右键断点 → "编辑断点"
- 输入条件，如：`i == 100`

**日志点**：
- 右键行号 → "添加日志点"
- 输入：`变量 i 的值: {i}`（不暂停，只打印）

### 调试控制台

在"调试控制台"标签页中可以：
- 输入表达式并执行
- 查看变量值
- 调用函数

---

