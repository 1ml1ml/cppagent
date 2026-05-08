# cppagent 项目流程指南

## 项目结构

```
cppagent/
├── lib/                  # cppagent_lib 静态库
│   ├── src/
│   │   ├── message/      # message 模块 (.ixx + .cpp)
│   │   ├── context/      # context 模块
│   │   ├── models/       # model_registry, standard_model
│   │   └── clients/      # i_client, openai_client
│   └── CMakeLists.txt
├── app/                  # 可执行文件
│   └── main.cpp
├── tests/                # 单元测试 (Catch2 v3)
│   ├── test_message.cpp
│   ├── test_context.cpp
│   ├── test_model_registry.cpp
│   ├── test_standard_model.cpp
│   └── test_openai_client.cpp
├── external/catch2       # Catch2 v3.14.0 源码
└── CMakeLists.txt
```

## 技术栈

- **编译器**: MSVC (Visual Studio 2022)
- **构建**: CMake 3.28+ + Ninja
- **标准**: C++20 Modules (.ixx)
- **包管理**: vcpkg (nlohmann_json, curl)
- **测试**: Catch2 v3.14.0
- **CI**: GitHub Actions (Windows)

---

## 开发流程

### 1. 环境准备 (一次性)

```powershell
# 确保已安装
# - Visual Studio 2022 (C++ 桌面开发)
# - CMake 3.28+
# - vcpkg (已配置 D:/vcpkg)
# - Git

# Clone 项目
git clone https://github.com/1ml1ml/cppagent.git
cd cppagent
```

### 2. 开发新功能

```
第 1 步：修改代码
  ↓
第 2 步：写/更新测试
  ↓
第 3 步：本地构建 + 跑测试
  ↓
第 4 步：提交 (commit + push)
  ↓
第 5 步：CI 自动跑测试
  ↓
第 6 步：GitHub 显示结果 (绿勾/红叉)
```

### 3. 本地开发命令

```powershell
# 生成项目 (VS 会自动做，也可以手动)
cmake -B out/build/x64-Debug -S . -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_TOOLCHAIN_FILE="D:/vcpkg/scripts/buildsystems/vcpkg.cmake"

# 构建全部
cmake --build out/build/x64-Debug --config Debug

# 只跑测试
cd out/build/x64-Debug
ctest -C Debug --output-on-failure
```

### 4. 提交规范

```
type: description

类型:
  feat:     新功能
  fix:      修复 bug
  test:     添加/修改测试
  refactor: 重构 (不改行为)
  docs:     文档更新
  chore:    杂项 (CI, 配置等)

示例:
  feat: add message attachment from_file support
  fix: handle empty context in openai_client
  test: add generate_async exception propagation test
```

---

## 测试流程

### 测试在哪里

- `tests/` 目录，和主代码一起存在于 `main` 分支
- **不做单独测试分支** — 测试是代码的一部分

### 如何写测试

```cpp
#include <catch2/catch_test_macros.hpp>

import message;

TEST_CASE("功能描述", "[模块名]")
{
  // Arrange
  auto msg{std::make_shared<message>(message::role::user, "hello"sv)};

  // Act
  msg->set_content("world"sv);

  // Assert
  REQUIRE(msg->get_content() == "world"sv);
}
```

### 测试要求

- 新功能 **必须** 带测试
- 修复 bug **优先** 先写复现测试，再修代码
- 所有测试在 CI 上 **必须** 通过才能合并

---

## CI/CD 流程

### GitHub Actions 自动触发条件

| 事件 | 动作 |
|------|------|
| `git push origin main` | 自动编译 + 跑测试 |
| Pull Request → main | 自动编译 + 跑测试 |

### CI 结果

- **绿勾 ✅**: 编译通过 + 所有测试通过
- **红叉 ❌**: 编译失败 或 测试挂了 → 发邮件通知
- 查看地址: `https://github.com/1ml1ml/cppagent/actions`

---

## 代码规范

### 命名

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | snake_case | `class message`, `class context` |
| 函数 | snake_case | `get_content()`, `set_name()` |
| 变量 | snake_case | `auto msg{...}`, `int count{0}` |
| 成员变量 | snake_case | `std::unique_ptr<impl> impl{}` |
| 枚举值 | SCREAMING_SNAKE_CASE | `role::UNKNOWN`, `role::USER` |

### 格式化

- **缩进**: 2 空格 (不用 tab)
- **初始化**: 统一 `{}`，禁止 `=` 和 `()`
- **PIMPL**: `unique_ptr<impl>`，必须显式声明析构函数
- **模块**: 导出用 `export module xxx;`，实现用 `module xxx;`

### 文件末尾

- 必须有且仅有 **1 个空行**

---

## 常见问题

### Q: 编译报错 "无法找到模块 xxx"
A: 在 VS 中右键 CMakeLists.txt → "删除缓存并重新配置"

### Q: 测试目标灰色
A: 确保 `enable_testing()` 在根 CMakeLists.txt，且 Catch2 编译完成

### Q: Catch2 和 C++20 Modules 冲突
A: 用完整源码版 Catch2 v3 (external/)，不是 amalgamated 单文件

### Q: 测试编译过但链接报错
A: 检查 `lib/CMakeLists.txt` 中 .cpp 是否用了 `PRIVATE` 而不是 `PUBLIC`

---

## 下一步建议

1. **GitHub Actions 验证**: 下次 push 代码看 CI 是否正常跑通
2. **添加更多测试**: 覆盖 edge case (空输入、异常路径)
3. **代码覆盖率**: 后续可以集成 codecov.io
4. **PR 模板**: 添加 `.github/pull_request_template.md`
