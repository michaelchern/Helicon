# 中文文档和英文 Agent 文档的工作流

这个仓库的文档分成两层：

- `docs/zh/`：给你看的中文源文档。
- `docs/`：给 AI Agent 看的英文上下文。

中文文档负责表达真实意图，英文文档负责让 AI 更快理解项目、查找文件、执行任务。英文文档不要求你手写，也不要求逐句对应中文；它应该由 AI 根据中文整理成更适合 Agent 使用的形式。

## 你的日常工作流

推荐按这个顺序工作：

1. 查看中文文档。
2. 了解项目功能和 AI 沉淀。
3. 校对是否有问题。
4. 修改中文文档。
5. 让 AI 根据中文修改英文文档。

换句话说，你的主要编辑面是中文文档。英文文档是 AI 消化中文之后生成的“执行版上下文”。

## 什么是 AI 沉淀

AI 沉淀不是单独某一个文件，而是让 AI 以后更稳定工作的上下文集合：

- `AGENTS.md`：Agent 工作总规则。
- `docs/`：英文 Agent 上下文。
- `docs/zh/`：中文源说明。
- `skills/`：固定工作流，例如同步文档、跑回归。
- `scripts/`：标准构建和测试入口。
- `tests/`：用测试固定重要行为。

你校对中文文档时，可以顺手检查这些沉淀是否还符合当前项目状态。

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

## 同步时 AI 应该怎么做

当你让 Codex 同步英文文档时，Codex 应该：

1. 先读对应中文文档，理解你的意图。
2. 再读对应英文文档，判断哪些规则需要更新。
3. 不逐句硬翻译，而是把中文意思整理成适合 AI 使用的英文上下文。
4. 保留命令、路径、API 名称、脚本名称、目标名称。
5. 如果中英文冲突，以中文为准。
6. 同步后说明改了哪些中文源和英文目标。

## 你可以怎么对 Codex 说

```text
我改了 docs/zh/project-structure.md，请按 AI 更好读取的方式同步 docs/project-structure.md。
```

```text
帮我检查中文文档和英文 Agent 文档有没有不一致，以中文为准同步英文。
```

```text
我想先改中文说明，你帮我把英文文档整理成更适合 AI 查找和执行的版本。
```

## 约定

- 中文文档适合人读，可以解释背景、原因和使用方式。
- 英文文档适合 AI 读，应该更短、更结构化、更偏规则和索引。
- 不要求中文和英文标题、段落、句子一一对应。
- 不要让用户承担英文整理工作；AI 负责把中文意图转换成英文 Agent 上下文。
- 如果项目目录、脚本、测试、工作流变化，先更新中文，再同步英文。
