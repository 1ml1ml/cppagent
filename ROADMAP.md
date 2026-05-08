# cppagent 项目蓝图 —— 从底向上逐层搭建

## 当前所处位置

**地基已打完（第 0 层完成），现在该搭第 1 层。**

---

## 第 0 层：基础设施（✅ 已完成）

### 0.1 数据模型
| 模块 | 状态 | 说明 |
|------|------|------|
| message | ✅ | role/content/attachment，支持文件附件 |
| context | ✅ | 消息容器，merge/append/clear/last_message |

### 0.2 抽象接口
| 模块 | 状态 | 说明 |
|------|------|------|
| i_model | ✅ | 模型配置接口（name/base_url/api_key/config） |
| standard_model | ✅ | PIMPL 实现，基于 nlohmann::json 存储配置 |
| model_registry | ✅ | 单例工厂注册表，provider 动态注册 |
| i_client | ✅ | generate / generate_async 接口 |
| openai_client | ✅ | liboai 对接，支持 temperature/top_p 等可选参数 |

### 0.3 工程支撑
| 模块 | 状态 | 说明 |
|------|------|------|
| Catch2 v3 | ✅ | 单元测试框架 |
| 测试覆盖 | ✅ | message/context/model_registry/standard_model/openai_client |
| CI (GitHub Actions) | ✅ | push/PR 自动编译+跑测试 |
| CMake 构建 | ✅ | lib/app/tests 三层结构 |

### 0.4 当前最大问题
**app/main.cpp 只有一个空壳** — 有零件但组装不起来。

---

## 第 1 层：配置管理（🔨 下一步要做）

### 为什么先做这个？
没有配置管理，所有参数都要硬编码（API key、模型名、base_url），项目无法在不同环境运行，也无法作为独立程序交付。

### 目标
```cpp
auto cfg = config_manager::load("cppagent.json");
cfg["api_key"]      // "sk-xxxx"
cfg["model"]        // "moonshot-v1-8k"
cfg["base_url"]     // "https://api.moonshot.cn/v1"
cfg["temperature"]  // 0.7
```

### 设计要点
- 支持 JSON 配置文件（`~/.config/cppagent/config.json` 或项目目录）
- 支持环境变量覆盖（`CPPGENT_API_KEY`）
- 支持默认值（temperature=0.7, max_tokens=2048）
- 配置校验（必填字段检查）
- 和 standard_model 的 config 对接

### 测试要求
- 加载完整配置
- 缺失必填字段抛异常
- 环境变量覆盖文件配置
- 默认值填充

---

## 第 2 层：对话生命周期管理（待做）

### 目标
管理一次**完整对话**的创建、进行、结束、保存。

```cpp
session_manager sm;
auto session = sm.create("uuid-1234");  // 新建会话
session->add_user_message("你好");
auto response = session->generate();       // 调用底层 client
session->save("sessions/uuid-1234.json"); // 持久化
```

### 职责划分
- **session**: 持有 context + 元数据（创建时间、标题、provider）
- **session_manager**: 管理多个 session 的 CRUD（创建/列表/加载/删除）
- **storage**: JSON 文件持久化（context → 文件，文件 → context）

### 为什么不是直接做 agent？
session 是"数据 + 状态"，agent 是"行为 + 编排"。先管好数据，再管行为。

---

## 第 3 层：Agent 核心引擎（待做）

### 目标
把配置 + 模型 + 客户端 + 对话 串成一条流水线。

```cpp
class agent {
public:
  static std::unique_ptr<agent> create(const nlohmann::json& config);

  // 一次对话交互
  message_shared_ptr chat(const std::string_view& user_input);

  // 多轮对话
  message_shared_ptr chat(const session_shared_ptr& session, const std::string_view& user_input);

  // 切换 provider
  void set_provider(const std::string_view& provider_name);

  // 获取当前对话历史
  context_shared_ptr current_context() const;

private:
  model_shared_ptr model_;
  client_unique_ptr client_;
  context_shared_ptr context_;  // 当前活跃对话
};
```

### 职责
- 根据配置创建 model + client
- 维护当前 context（多轮对话状态）
- chat() 内部：user_input → message → context.append → client.generate → context.append(response)
- 异常处理（网络失败、API 错误透传）

---

## 第 4 层：应用外壳（待做）

### CLI 交互循环
```cpp
int main() {
  auto cfg = config_manager::load_or_create();
  auto ag = agent::create(cfg);

  while (true) {
    std::cout << "> ";
    std::string input;
    std::getline(std::cin, input);
    if (input == "/quit") break;

    auto response = ag->chat(input);
    std::cout << response->get_content() << "\n";
  }
}
```

### 命令体系
| 命令 | 功能 |
|------|------|
| `/quit` | 退出 |
| `/new` | 新建会话 |
| `/list` | 列出历史会话 |
| `/load <id>` | 加载会话 |
| `/provider <name>` | 切换 provider |
| `/config` | 查看当前配置 |

---

## 第 5 层：扩展能力（远期）

### 5.1 工具调用（function calling）
- 工具注册表（tool_registry）
- 工具描述自动生成（传给 LLM 的 function schema）
- 工具执行器（根据 LLM 返回的 function_call 执行本地函数）
- 执行结果回传（observation → context → 下一轮 generate）

### 5.2 多模态
- 图片输入（vision model）
- 文件上传（PDF/代码分析）

### 5.3 高级功能
- 流式输出（实时打字机效果）
- 会话标题自动生成（基于第一轮对话让 LLM 总结）
- 插件系统（动态加载 so/dll）

---

## 当前精确位置

```
第 0 层  ████████████████████  ✅ 完成
第 1 层                      🔨 现在要做
第 2 层  
第 3 层  
第 4 层  
第 5 层  
```

**结论：地基已打好，现在应该造 "配置管理"（第 1 层），而不是直接跳到 agent。**

没有配置管理，agent 创建时参数没法传入，main.cpp 永远只能硬编码。
