# 项目结构和 AI 沉淀说明

这份文档是当前工作树的简化地图，用来帮助你和 AI 快速对齐上下文。

## 当前使用方式

- 这个仓库现在更像一个“候选代码整理区”，不是已经定版的工程骨架。
- 推荐工作流是：挑代码、复制进仓库、讨论取舍、继续重组、把稳定结论写进文档。
- 文档的目标是让后续更快接上，不是维护一份过度理想化的蓝图。

## 当前目录树

```text
Helicon/
|-- .codex/
|-- .github/
|-- docs/
|   `-- zh/
|-- examples/
|   `-- triangle_graph/
|-- include/
|   |-- common/
|   |-- rhi.h
|   |-- rhiHLSL.h
|   |-- utils.h
|   |-- validation.h
|   `-- vulkan.h
|-- scripts/
|-- skills/
|-- src/
|   |-- ast/
|   |-- backends/
|   |   |-- cuda/
|   |   `-- vulkan/
|   |       `-- vulkan_backend.cpp
|   `-- dsl/
|-- tests/
|-- third_party/
|-- AGENTS.md
|-- CMakeLists.txt
|-- README.md
`-- README-zh.md
```

`build/`、`out/`、`.vs/` 和 `.git/` 都属于生成目录或本地状态，不要手动维护其中产物。

## 目录职责

| 路径 | 当前含义 |
| --- | --- |
| `AGENTS.md` | 仓库级 Agent 规则和工作方式。 |
| `docs/zh/` | 给人看的中文源说明。 |
| `docs/` | 给 Agent 用的英文执行上下文。 |
| `include/` | 当前保留的公开头文件候选区。 |
| `include/common/` | 共用底层类型、容器或工具的候选区。 |
| `include/utils.h` | 当前仓库的参考头文件；文件头模板、双语注释和辅助函数蛇形命名优先向它对齐。 |
| `include/rhiHLSL.h` | C++/HLSL 共享兼容头；统一注释与模板，但默认保留受上游、规范和 shader 约束的命名。 |
| `src/` | 当前实现和实验代码区域。 |
| `src/backends/vulkan/` | 当前保留的 Vulkan 侧实现。 |
| `src/ast/`、`src/dsl/`、`src/backends/cuda/` | 预留或占位目录；不要默认它们已经有稳定职责。 |
| `examples/triangle_graph/` | 示例或临时验证入口。 |
| `tests/` | 未来稳定后使用的验证目录；当前可以为空。 |
| `scripts/` | 仍然值得保留的脚本入口。 |
| `skills/` | 仓库内沉淀的 AI 工作流。 |
| `third_party/` | 第三方代码预留区。 |

## 如何判断什么算“真实结构”

- 优先看实际文件和最新文档，不要从旧 README、旧 CMake 或旧测试假设反推当前状态。
- 只有 `.gitkeep` 的目录一律视为预留，不视为已落地模块。
- 如果 `CMakeLists.txt`、脚本和工作树不一致，先尊重当前工作树和你的意图，再决定是否恢复构建。

## 建议沉淀什么

- 哪些模块决定留下。
- 哪些命名和目录边界已经定了。
- 哪些依赖值得继续保留。
- 哪些构建/测试约定已经恢复为真实流程。
- 代码格式化、命名和双语注释应优先遵循什么统一规则。
