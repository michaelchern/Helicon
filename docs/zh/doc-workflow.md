# 中文文档和英文 Agent 文档的工作流

## 分工

- `docs/zh/`：给人看的中文源文档。
- `docs/`：给 Agent 用的英文执行上下文。

中文负责表达真实意图，英文负责让 Agent 更快查找、判断和执行。

## 当前沉淀原则

- 只沉淀已经稳定下来的决定。
- 明显过期、重复或误导的内容可以直接删减，不必为了“保留历史”继续挂着。
- 用户不负责写英文，AI 负责把中文整理成更适合 Agent 使用的版本。

## 推荐流程

1. 先改中文文档。
2. 再同步对应英文文档。
3. 如果目录、构建、测试或工作方式变了，把相关文档一起收敛。

## 对应关系

| 中文源文档 | 英文 Agent 文档 |
| --- | --- |
| `docs/zh/architecture.md` | `docs/architecture.md` |
| `docs/zh/build.md` | `docs/build.md` |
| `docs/zh/testing.md` | `docs/testing.md` |
| `docs/zh/project-structure.md` | `docs/project-structure.md` |
| `docs/zh/github-workflow.md` | `docs/github-workflow.md` |
| `docs/zh/doc-workflow.md` | `docs/doc-sync.md` |
| `README-zh.md` | `README.md` |

## 同步规则

- 先读中文，再改英文。
- 不逐句硬翻译，而是重组成更适合 Agent 的规则和索引。
- 保留命令、路径、脚本名和目录名。
- 如果中英文冲突，以中文为准。
- 同步时顺手清掉旧假设和重复说明。
