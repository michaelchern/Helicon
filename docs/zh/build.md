# 构建说明

## 前置要求

- CMake `>= 3.20`
- 一个能被 CMake 找到的 C++20 编译工具链
- Windows 上推荐安装 Visual Studio Build Tools 或带 C++ 工作负载的 Visual Studio
- Vulkan SDK，用于编译和链接 Vulkan 后端

## 标准构建命令

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

## 构建脚本做什么

1. 在 `build/` 目录配置 CMake。
2. 查找 Vulkan SDK。
3. 构建 `helicon` 静态库。
4. 构建 `helicon_tests` C++ smoke 测试程序。
5. 构建 `helicon_render_graph_tests` C++ render graph 测试程序。
6. 默认构建 `helicon_triangle_graph` 示例。

## 预期产物

- `build/` 下生成 `helicon` 静态库。
- `build/` 下生成测试可执行文件。
- `build/` 下生成三角形示例可执行文件。

## 常见失败

- 找不到 `cmake`：安装 CMake，并确认它在 `PATH` 里。
- 找不到编译器：安装受支持的 C++20 工具链。
- 找不到 Vulkan：安装 Vulkan SDK，并确认 CMake 能找到 `Vulkan::Vulkan`。
- 目标或源文件不存在：检查 `CMakeLists.txt` 和真实目录是否一致。
