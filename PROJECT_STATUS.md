# cppagent 项目框架梳理

## 当前项目分层架构

```
┌─────────────────────────────────────────┐
│  app/                                   │
│  ├─ main.cpp (❌ 基本为空)              │
│  └─ CMakeLists.txt                      │
├─────────────────────────────────────────┤
│  lib/                                   │
│  ├─ src/                                │
│  │   ├─ message/     ✅ message.ixx/cpp   │
│  │   ├─ context/     ✅ context.ixx/cpp  │
│  │   ├─ models/                          │
│  │   │   ├─ i_model.ixx          ✅      │
│  │   │   ├─ model_registry.ixx/cpp ✅    │
│  │   │   └─ standard_model.ixx/cpp ✅    │
│  │   └─ clients/                         │
│  │       ├─ i_client.ixx         ✅      │
│  │       └─ openai_client.ixx/cpp  ✅      │
│  └─ CMakeLists.txt                        │
├─────────────────────────────────────────┤
│  tests/                                   │
│  ├─ test_message.cpp       ✅            │
│  ├─ test_context.cpp       ✅            │
│  ├─ test_model_registry.cpp ✅           │
│  ├─ test_standard_model.cpp  ✅           │
│  └─ test_openai_client.cpp   ✅           │
├─────────────────────────────────────────┤
│  external/catch2/          ✅            │
├─────────────────────────────────────────┤
│  .github/workflows/ci.yml  ✅            │
└─────────────────────────────────────────┘
```

---

## 各层完成度

### 1. 数据层（domain） ✅ 完成
- **message**: role/content/attachment，支持文件附件
- **context**: 消息容器，支持 merge/append/clear

### 2. 抽象层（abstraction） ✅ 完成
- **i_model**: 模型配置接口（config/name/base_url/api_key）
- **model_registry**: 单例工厂注册表，支持 provider 动态注册
- **standard_model**: PIMPL 实现，基于 nlohmann::json 配置
- **i_client**: generate / generate_async 接口
- **openai_client**: 对接 liboai，支持可选参数 + stream_callback

### 3. 测试层（testing） ✅ 完成
- Catch2 v3.14.0 集成
- 5 个模块全部覆盖单元测试
- CI: GitHub Actions 自动编译 + 跑测试

### 4. 应用层（application） ❌ 空缺
- `main.cpp` 只有辅助函数和硬编码 API key，没有业务逻辑

---

## 当前项目所处的阶段

**阶段 1: 基础框架搭建 → ✅ 已完成**
- C++20 Modules 项目结构
- 核心领域模型 + 抽象接口
- 测试 + CI 流程

**阶段 2: Agent 核心引擎 → ❌ 还没开始**
- 对话循环（多轮对话管理）
- 配置管理（环境变量 / 配置文件）
- 消息流转：user_input → context → client.generate → response → context.append

**阶段 3: 应用层（CLI / GUI）→ ❌ 还没开始**
- 交互式命令行
- 会话管理（新建/切换/保存对话）
- 多 provider 切换

**阶段 4: 扩展能力 → ❌ 还没开始**
- 工具调用（function calling）
- 文件上传 / 图片输入
- 插件系统

---

## 明显的空缺

| 空缺项 | 当前状态 | 影响 |
|--------|---------|------|
| app/main.cpp 为空 | ❌ | 项目无法作为应用程序运行 |
| 没有配置管理 | ❌ | API key / model 硬编码，无法部署 |
| 没有对话循环 | ❌ | 只能单次调用，没有多轮对话 |
| 没有 Agent 类 | ❌ | 没有统一入口把各模块串起来 |
| 没有会话持久化 | ❌ | 对话历史无法保存/恢复 |
| function_call 只传参数未实现 | ⚠️ | openai_client 支持传 function_call，但没有工具注册和执行 |

---

## 下一步建议

**最自然的下一步：实现 Agent 核心类**

需要一个 `agent` 类把各模块串联起来：

```
┌──────────┐     ┌──────────┐     ┌────────────┐
│  agent   │────▶│ context  │────▶│  i_client  │
│ (编排层)  │     │ (数据)    │     │ (通信层)    │
└──────────┘     └──────────┘     └────────────┘
      │                                 │
      ▼                                 ▼
┌──────────┐                    ┌──────────────┐
│ i_model  │                    │ openai_client │
│ (配置)   │                    │ (liboai)      │
└──────────┘                    └──────────────┘
```

agent 类的职责：
- 持有 model + client + context
- 提供 `chat(user_input) -> response` 接口
- 自动把 user 消息 append 到 context
- 调用 client.generate，把 assistant 回复 append 到 context
- 支持多轮对话

然后 main.cpp 变成：
```cpp
auto cfg = load_config("config.json");
auto agent = agent::create(cfg);

while (true) {
    auto input = read_line("> ");
    auto response = agent.chat(input);
    std::cout << response << "\n";
}
```

---

## 近期任务优先级

1. **高**: `agent` 核心类（串联所有模块，实现对话循环）
2. **高**: `config` 管理（JSON 配置文件读取，替代硬编码）
3. **中**: `session` 持久化（对话历史保存到文件）
4. **中**: CLI 交互循环（命令行输入/输出）
5. **低**: 工具调用（function calling）

**结论：基础设施全部就绪，现在缺的是"把零件组装成能跑的车"。下一步应该实现 agent 核心引擎。**
