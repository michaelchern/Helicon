# 架构说明

## 目标

这个仓库目前保持最小、清晰、可运行。这样做是为了让人和 Codex 都能用较少上下文理解项目，并且用小步改动持续迭代。

## 目录职责

- `include/`：公开头文件，也是稳定 API 边界。
- `src/`：生产代码实现，当前按 `ast/`、`dsl/`、`backends/` 预留边界。
- `tests/`：可执行测试。
- `scripts/`：标准构建和测试入口。
- `docs/`：给 Agent 使用的英文文档。
- `docs/zh/`：给人阅读和修改的中文源文档。
- `examples/`：示例和最小用法演示的预留目录。
- `third_party/`：第三方代码或外部依赖的预留目录。

更详细的目录和脚本职责见 `docs/zh/project-structure.md`。

## 构建和测试流程

1. `scripts/build.ps1` 使用 CMake 配置并构建到 `build/`。
2. `scripts/test.ps1` 在同一个 `build/` 目录里运行 CTest。

## API 边界

公开 API 放在 `include/` 下。除非确实需要破坏兼容性，否则优先扩展已有接口，而不是重命名或删除公开接口。

## 改动原则

- 只改解决问题所需的最小范围。
- 行为或流程变化时同步更新文档。
- 构建、测试、CI 优先调用 `scripts/` 下的脚本。
