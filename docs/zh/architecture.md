# 架构说明

## 目标

Helicon 当前定位为 **纯 C++20 高性能渲染核心**。第一阶段聚焦 Windows + Vulkan，
目标是用最小 RHI 和 render graph 跑通无窗口离屏三角形渲染。

选择 C++20 作为项目标准：它已经提供当前需要的 `std::span`、RAII、强类型接口和现代标准库能力；
工具链支持成熟；C++23 暂时不会明显降低 v0 实现复杂度。

暂不做窗口系统、PBR、glTF、ray tracing、async compute、多图形 API、CUDA/Metal/DirectX
后端，也暂不实现 shader DSL。Shader DSL 会在渲染链路稳定后，再基于 `src/ast/`、
`src/dsl/` 和 `src/backends/` 扩展。

RHI 框架采用“稳定公共抽象 + Vulkan-first 后端 + 能力扩展”的方向：公共 API
保持后端无关，不能暴露 Vulkan 句柄；v0 只实现 Vulkan，但 Device、Queue、资源、
Binding、Pipeline、Framebuffer 等边界要保留多后端余地。光追、CUDA interop、
bindless、native handle 等高级能力后续通过 capability/extension 查询进入，
不要直接塞进基础 RHI 对象。

详细技术路线、参考仓库路径和吸收顺序属于本地私有 Agent 记忆，放在
`.codex/private/` 下并由 `.gitignore` 保护；正式文档只沉淀可公开的稳定原则。

## 目录职责

- `include/helicon/`：C++ 公共 API。当前主入口是 `helicon/helicon.hpp`。
- `src/`：生产代码实现，包括 RHI façade、render graph 和后端实现。
- `src/backends/vulkan/`：Vulkan v0 后端，负责实例/设备/队列、资源、命令、离屏渲染和读回。
- `src/shaders/`：内置 shader 数据。当前保存预编译三角形 SPIR-V 常量。
- `src/ast/`、`src/dsl/`：后续 Helicon Shader DSL 的预留边界。
- `tests/`：C++ smoke 测试和 C++ render graph 回归测试。
- `examples/`：可运行示例。当前有 `triangle_graph`。
- `scripts/`：标准构建和测试入口。
- `docs/zh/`：中文源文档。
- `docs/`：英文 Agent 上下文。
- `.codex/private/`：本地私有 Agent 计划和路线，不进入 Git。
- `third_party/`：第三方代码预留区；新增依赖前必须有明确理由。

## 构建和测试流程

1. `scripts/build.ps1` 使用 CMake 配置并构建到 `build/`。
2. `scripts/test.ps1` 在同一个 `build/` 目录里运行 CTest。

标准验证：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

## API 边界

公开 API 放在 `include/helicon/` 下。Vulkan 句柄不能暴露到公共 API。
后端细节必须留在 `src/backends/vulkan/`。基础 RHI 只表达通用资源、命令、
绑定、管线、帧缓冲和能力查询；后端特有能力通过 extension 边界暴露给内部模块。

## 改动原则

- 优先做小步、可验证的改动。
- 行为、目录、依赖或流程变化时同步更新中文文档，再同步英文 Agent 文档。
- 构建、测试、CI 优先调用 `scripts/` 下的脚本。
- 新增依赖要先证明能减少真实复杂度；当前 Vulkan 后端只依赖 Vulkan SDK。
