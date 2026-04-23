# 测试说明

## 标准测试命令

```powershell
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

如果构建产物不存在或可能过期，先运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

## 当前测试策略

- `tests/helicon_smoke_test.cpp`：C++ smoke 测试，验证版本、状态字符串和后端可用性查询。
- `tests/render_graph_test.cpp`：C++ RHI/render graph 回归测试。Vulkan 后端可用时会创建 device、buffer、image、
  command list，执行 clear-only pass 和内置三角形 pass，并检查 RGBA8 readback 像素。
- 如果机器没有可用 Vulkan 设备，C++ GPU 回归会跳过 GPU 部分并通过；构建仍需要 Vulkan SDK。

## 通过标准

- `scripts/build.ps1` 成功。
- `scripts/test.ps1` 显示所有 CTest 测试通过。

## 排查流程

1. 重新运行 `scripts/build.ps1`。
2. 重新运行 `scripts/test.ps1`。
3. 先看第一个失败测试的输出。
4. 如果是 Vulkan 初始化失败，先确认 Vulkan SDK、驱动和 GPU/软件 Vulkan 设备是否可用。
5. 只修改最小受影响代码区域。
