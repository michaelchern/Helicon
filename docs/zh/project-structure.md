# 项目结构和 AI 沉淀说明

这份文档是 Helicon 的项目地图。中文文档是给人看的源说明，英文 `docs/`
是 Codex 使用的执行版上下文。

## 建议先看什么

1. `README-zh.md`：项目入口和快速开始。
2. `docs/zh/architecture.md`：项目目标、边界和核心约定。
3. `docs/zh/project-structure.md`：当前目录和 AI 沉淀说明，也就是本文。
4. `docs/zh/build.md`：如何构建。
5. `docs/zh/testing.md`：如何测试。
6. `docs/zh/doc-workflow.md`：中文文档和英文 Agent 文档如何同步。

## 当前目录树

```text
Helicon/
|-- .codex/
|-- .github/
|   `-- workflows/
|-- docs/
|   `-- zh/
|-- examples/
|   `-- triangle_graph/
|-- include/
|   `-- helicon/
|       `-- helicon.hpp
|-- scripts/
|-- skills/
|   |-- prepare-github-pr/
|   |-- run-regression/
|   `-- sync-docs/
|-- src/
|   |-- ast/
|   |-- backends/
|   |   |-- cuda/
|   |   `-- vulkan/
|   |-- dsl/
|   |-- shaders/
|   |-- render_graph.cpp
|   |-- rhi.cpp
|   `-- rhi_internal.hpp
|-- tests/
|   |-- helicon_smoke_test.cpp
|   `-- render_graph_test.cpp
|-- third_party/
|-- .clang-format
|-- .gitignore
|-- AGENTS.md
|-- CMakeLists.txt
|-- README.md
`-- README-zh.md
```

`build/`、`out/` 是 CMake 或 IDE 自动生成的构建目录，`.vs/` 是 Visual Studio
本地工作目录，`.git/` 是 Git 内部目录。不要手动维护这些目录里的生成产物。

## 目录职责

| 路径 | 它是干什么的 |
| --- | --- |
| `include/helicon/helicon.hpp` | C++20 主 API。定义 RHI façade、render graph 类型和错误类型。 |
| `src/rhi.cpp` | C++ API façade 到后端接口的实现。 |
| `src/render_graph.cpp` | 最小 render graph v0，实现 image/pass 声明、执行和读回。 |
| `src/rhi_internal.hpp` | 内部后端接口，不能作为公共 API 使用。 |
| `src/backends/vulkan/` | Vulkan 后端实现。公共 API 不暴露这里的 Vulkan 句柄。 |
| `src/shaders/` | 内置预编译 shader 数据。 |
| `src/ast/`、`src/dsl/` | Helicon Shader DSL 后续阶段预留。 |
| `examples/triangle_graph/` | 无窗口三角形示例，输出 `triangle_graph.ppm`。 |
| `tests/` | C++ smoke 和 C++ render graph 回归测试。 |
| `scripts/` | 标准脚本入口，构建和测试优先用这里。 |
| `skills/` | 仓库内的 AI 工作流沉淀。 |
| `docs/zh/` | 中文源文档。 |
| `docs/` | 英文 Agent 上下文，由中文同步而来。 |
| `third_party/` | 第三方代码预留；新增依赖需要明确理由。 |

## AI 沉淀放在哪里

| 位置 | 沉淀什么 |
| --- | --- |
| `AGENTS.md` | Agent 在这个仓库工作的总规则。 |
| `docs/zh/` | 人维护的中文源说明。 |
| `docs/` | AI 读取的英文执行上下文。 |
| `skills/sync-docs/` | 中文到英文 Agent 文档同步工作流。 |
| `skills/run-regression/` | 标准构建和测试工作流。 |
| `skills/prepare-github-pr/` | 提交和 PR 草稿工作流。 |
| `scripts/` | 人、AI、CI 共用的命令入口。 |
| `tests/` | 用测试固定重要行为。 |

## 脚本是干什么的

| 脚本 | 作用 |
| --- | --- |
| `scripts/build.ps1` | 标准构建入口：检查 `cmake`，在 `build/` 配置并构建项目。 |
| `scripts/test.ps1` | 标准测试入口：检查 `ctest`，确认 `build/` 存在，然后运行 CTest。 |

## 维护约定

- 项目使用 C++20，不再保留 C 源文件或 C ABI。
- 目录职责要尽量单一：接口放 `include/helicon/`，实现放 `src/`，测试放 `tests/`，脚本放 `scripts/`。
- Vulkan 后端细节留在 `src/backends/vulkan/`；不要泄露 Vulkan 句柄到公共 API。
- 行为、目录、依赖或流程变化时，先更新中文文档，再同步英文文档。
- 不要手动修改 `build/`、`out/`、`.vs/` 或 `.git/`。
