# 项目结构和 AI 沉淀说明

这份文档是给你看的项目地图。它的作用不是把每个技术细节都讲完，而是让你打开仓库时能快速知道：

- 这个项目目前有哪些目录。
- 每个目录大概放什么。
- 哪些内容是项目功能，哪些内容是 AI 协作沉淀。
- 发现说明不准时，应该改哪里。

英文文档不是人工主入口。英文文档主要给 AI Agent 读取，用来压缩上下文、快速查找和减少误改。你只需要优先维护中文文档，再让 AI 根据中文同步英文文档。

## 你的文档工作流

推荐按这个顺序使用文档：

1. 查看中文文档：从 `README-zh.md` 和 `docs/zh/` 开始。
2. 了解项目功能和 AI 沉淀：看项目目录、脚本、测试、技能说明是否符合当前想法。
3. 校对是否有问题：发现目录用途、流程、约定不准确，就先在中文文档里改清楚。
4. 修改中文文档：中文表达你的真实意图，不用先写英文。
5. 让 AI 同步英文文档：告诉 Codex “根据中文同步英文文档”，AI 会把中文意思整理成适合 Agent 使用的英文上下文。

## 建议先看什么

第一次进入仓库时，可以按这个顺序看：

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
|-- include/
|   `-- helicon.h
|-- scripts/
|-- skills/
|   |-- prepare-github-pr/
|   |-- run-regression/
|   `-- sync-docs/
|-- src/
|   |-- helicon.c
|   |-- ast/
|   |-- backends/
|   |   |-- cuda/
|   |   `-- vulkan/
|   `-- dsl/
|-- tests/
|   `-- helicon_test.c
|-- third_party/
|-- .clang-format
|-- .gitignore
|-- AGENTS.md
|-- CMakeLists.txt
|-- README.md
`-- README-zh.md
```

`build/`、`out/` 是 CMake 或 IDE 自动生成的构建目录，`.vs/` 是 Visual Studio 本地工作目录，`.git/` 是 Git 内部目录。它们不是你平时维护项目结构时需要改的地方，也不应该当成项目源码提交。

## 人应该怎么看这些目录

| 路径 | 它是干什么的 | 你什么时候会关心它 |
| --- | --- | --- |
| `docs/zh/` | 中文源文档。这里记录你真正想表达的项目说明、流程和约定。 | 你想校对项目说明、修改约定、沉淀想法时，优先改这里。 |
| `docs/` | 英文 Agent 文档。它是 AI 用的压缩上下文，不要求你手写。 | 你通常只需要检查 AI 同步后有没有误解中文意思。 |
| `src/` | 生产代码实现。 | 要实现功能、改行为、拆内部模块时看这里。 |
| `include/` | 对外公开头文件，也就是 API 边界。 | 要新增或调整别人会调用的接口时看这里。 |
| `tests/` | 测试代码。 | 改功能、修 bug 后，用这里验证行为是否正确。 |
| `scripts/` | 标准脚本入口。 | 构建、测试、回归验证时，优先用这里的脚本。 |
| `examples/` | 示例和最小用法演示的预留目录。 | 想给自己或别人展示“怎么用”时，把示例放这里。 |
| `skills/` | 仓库内的 AI 技能说明。 | 某些重复工作流想让 AI 稳定执行时，把方法沉淀到这里。 |
| `.github/workflows/` | GitHub Actions 自动化配置。 | CI、PR 检查、自动构建和测试出问题时看这里。 |
| `.codex/` | Codex 本地配置和预留策略。 | 只有需要调整 Codex 行为时才看。 |
| `third_party/` | 第三方代码或外部依赖的预留目录。 | 真正需要引入外部代码时再使用。 |

## `src/` 里的功能边界

这些目录目前主要是为未来功能预留边界：

| 路径 | 准备承载什么 |
| --- | --- |
| `src/helicon.c` | 当前最小生产实现入口。 |
| `src/ast/` | 抽象语法树相关内容，例如节点结构、遍历、语义表示。 |
| `src/dsl/` | DSL 前端相关内容，例如语法解析、输入转换、诊断信息。 |
| `src/backends/` | 不同后端共享的实现逻辑。 |
| `src/backends/cuda/` | CUDA 后端相关实现。 |
| `src/backends/vulkan/` | Vulkan 后端相关实现。 |

这些目录现在可以只有 `.gitkeep` 占位文件。`.gitkeep` 只是为了让 Git 保存空目录，不代表功能已经完成。

当前公开头文件入口是 `include/helicon.h`，当前测试入口是 `tests/helicon_test.c`。

## AI 沉淀放在哪里

这里的“AI 沉淀”指的是：为了让以后 AI 更稳定地理解项目、复用经验、少走弯路而留下的上下文。

| 位置 | 沉淀什么 |
| --- | --- |
| `AGENTS.md` | Agent 在这个仓库工作的总规则，例如先读哪些文档、如何验证。 |
| `docs/` | 给 AI 读取的英文上下文。它应该短、清楚、可执行。 |
| `docs/zh/` | 给你维护的中文源文档。它可以更解释性、更像给人看的说明。 |
| `skills/sync-docs/` | “根据中文同步英文文档”的固定工作流。 |
| `skills/run-regression/` | “运行标准构建和测试”的固定工作流。 |
| `skills/prepare-github-pr/` | “先生成 GitHub 提交和 PR 草稿，等你确认后再提交”的固定工作流。 |
| `scripts/` | 让人、AI、CI 都使用同一套命令入口。 |
| `tests/` | 用测试把重要行为固定下来，避免以后误改。 |

## 脚本是干什么的

| 脚本 | 作用 |
| --- | --- |
| `scripts/build.ps1` | 标准构建入口：检查 `cmake`，在 `build/` 配置并构建项目。 |
| `scripts/test.ps1` | 标准测试入口：检查 `ctest`，确认 `build/` 存在，然后运行 CTest。 |

以后如果新增脚本，建议同时写清楚三个问题：

- 什么时候运行它。
- 它需要什么输入。
- 它会产生什么结果。

## 发现文档不准时怎么办

如果你发现项目说明和真实目录不一致，可以按这个顺序处理：

1. 先改对应的中文文档，例如本文或 `docs/zh/architecture.md`。
2. 如果改动影响构建、测试或使用方式，也同步改 `docs/zh/build.md` 或 `docs/zh/testing.md`。
3. 让 Codex 根据中文同步英文文档。
4. 如果涉及代码行为，再补测试并运行 `scripts/build.ps1` 和 `scripts/test.ps1`。

## 维护约定

- 你主要维护中文文档，英文文档由 AI 根据中文整理。
- 中文文档要适合人读，可以解释背景和原因。
- 英文文档要适合 AI 读，要短、明确、可执行。
- 目录职责要尽量单一：接口放 `include/`，实现放 `src/`，测试放 `tests/`，脚本放 `scripts/`。
- 不要手动修改 `build/` 和 `.git/`，也不要把生成产物当成源码提交。
