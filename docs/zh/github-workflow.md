# GitHub 提交和 PR 工作流

这份文档记录你后面希望使用 Codex 提交到 GitHub 的方式：

1. Codex 先检查当前改动。
2. Codex 生成提交总结、提交信息、PR 标题和 PR 描述草稿。
3. 你校对这些内容。
4. 你确认后，Codex 才执行提交、推送和创建 PR。

核心原则：先给你看清楚，再让 GitHub 上产生真实动作。

## 推荐流程

### 1. 让 Codex 准备草稿

你可以这样说：

```text
帮我准备提交到 GitHub 的总结和 PR 描述，我校对后再提交。
```

Codex 应该先做这些事：

- 查看 `git status`，确认当前有哪些改动。
- 查看 diff，理解改动范围。
- 如果改动包含代码，优先确认是否已经运行 `scripts/build.ps1` 和 `scripts/test.ps1`。
- 生成给你校对的内容，不直接提交。

### 2. Codex 给出校对内容

草稿至少应该包含：

- 改动总结。
- 为什么需要这个改动。
- 验证情况。
- 风险或后续事项。
- 建议的 commit message。
- 建议的 PR title。
- 建议的 PR description。
- 准备纳入提交的文件列表。

### 3. 你校对并确认

你可以要求 Codex 修改草稿，例如：

```text
PR 描述再写得短一点。
```

```text
这个风险不用写，改成后续优化。
```

```text
确认，按这个提交。
```

只有你明确确认之后，Codex 才可以继续执行真实提交动作。

### 4. Codex 执行提交和 PR

确认后，Codex 才可以：

1. 只 stage 本次确认范围内的文件。
2. 创建 commit。
3. push 到 GitHub 分支。
4. 创建 draft PR。
5. 返回 PR 链接、分支名、commit、验证结果。

默认创建 draft PR，除非你明确要求直接 ready for review。

## 安全约定

- Codex 不应该在你校对前 commit、push 或创建 PR。
- 如果工作区里有不相关改动，Codex 应该先列出来，让你决定哪些要提交。
- 默认不要使用 `git add -A`，除非确认整个工作区都属于本次提交。
- 如果验证失败，Codex 应该先说明失败原因和建议，不要继续 push。
- 如果 GitHub 登录、remote 或权限有问题，Codex 应该停下来说明阻塞点。

## AI 沉淀位置

- 固定流程写在 `skills/prepare-github-pr/`。
- Agent 执行版规则写在 `docs/github-workflow.md`。
- 你平时看这份中文文档即可。
