# 构建说明

## 前置要求

- CMake `>= 3.20`
- 一个能被 CMake 找到的 C 编译工具链
- Windows 上推荐安装 Visual Studio Build Tools 或带 C++ 工作负载的 Visual Studio

## 标准构建命令

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

## 构建脚本做什么

1. 在 `build/` 目录配置 CMake。
2. 构建 `helicon` 静态库。
3. 构建 `helicon_tests` 测试程序。

## 预期产物

- `build/` 下生成静态库（具体子目录取决于工具链）。
- `build/` 下生成测试可执行文件。

## 常见失败

- 找不到 `cmake`：安装 CMake，并确认它在 `PATH` 里。
- 找不到编译器：安装受支持的 C 工具链。
