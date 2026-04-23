<p align="center">
  <a href="./README.md">English</a> | <a href="./README-zh.md">简体中文</a>
</p>

# Helicon

Helicon 是一个以 Codex 协作为主的 **纯 C++20** 高性能渲染核心。当前阶段聚焦
Windows + Vulkan：提供 C++ RHI、最小 render graph 和无窗口离屏三角形渲染。

选择 C++20 的原因：

- 已满足当前 API 需要，例如 `std::span`、RAII、强类型枚举和现代标准库能力。
- Visual Studio、CMake 和主流编译器支持成熟，适合现阶段迭代。
- C++23 暂时不会明显简化 v0 渲染核心，反而会提高工具链门槛。

当前 v0 能力：

- C++ API：`helicon::Context`、`Device`、`Queue`、`Buffer`、`Image`、`ShaderModule`、
  `GraphicsPipeline`、`CommandList`。
- Render graph v0：声明 RGBA8 color attachment、添加 pass、写入 color、执行内置三角形。
- Vulkan 后端：无窗口离屏渲染、基础资源创建、命令提交、RGBA8 读回。
- 示例：`helicon_triangle_graph` 会渲染三角形并输出 `triangle_graph.ppm`。

## 快速开始（Windows / PowerShell）

需要 CMake、Visual Studio C++ 工具链和 Vulkan SDK。

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

## 仓库文档

- `docs/zh/architecture.md`
- `docs/zh/project-structure.md`
- `docs/zh/build.md`
- `docs/zh/testing.md`
- `docs/zh/github-workflow.md`
- `docs/zh/doc-workflow.md`

中文文档是人的主入口和源说明；英文 `docs/` 是 Codex 使用的执行版上下文。
