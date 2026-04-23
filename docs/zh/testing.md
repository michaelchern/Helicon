# 测试说明

## 标准测试命令

```powershell
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

## 当前测试策略

- 测试入口在 `tests/helicon_test.c`。
- 测试通过 CTest 执行，这样本地和 CI 可以使用同一套入口。

## 通过标准

- `scripts/build.ps1` 成功。
- `scripts/test.ps1` 显示所有测试通过。

## 排查流程

1. 重新运行 `scripts/build.ps1`。
2. 重新运行 `scripts/test.ps1`。
3. 先看第一个失败测试的输出。
4. 只修改最小受影响代码区域。
