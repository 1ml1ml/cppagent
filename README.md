# cppagent

基于 C++20 Modules 的 LLM Agent 框架，支持 MCP (Model Context Protocol)。

## 特性

- **C++20 Modules** — 使用 `.ixx` 接口单元，现代模块化 C++
- **多厂商 LLM** — 通过 [ai-sdk-cpp](https://github.com/ClickHouse/ai-sdk-cpp) 实现可插拔的 Provider 注册表
  - 支持 OpenAI、Anthropic、OpenRouter、Moonshot、Kimi 等兼容 OpenAI API 的服务
  - 自动多步 tool calling（框架自动处理 tool → result → reply 循环）
- **MCP 客户端** — 完整的 Model Context Protocol 客户端实现
  - JSON-RPC 2.0 over stdio 传输
  - `mcp_manager` 多服务器管理
  - Tool 发现与调用，带 server 前缀命名空间（`server/tool`）
  - Resource 列表与读取
- **配置管理** — `app_config` 从 JSON 文件统一读取模型配置和 MCP 服务器配置
- **CMake 3.28+**
- **Windows 为主** — `app` 使用 Windows 控制台 API；核心库可在 Linux/WSL 编译

## 项目结构

```
cppagent/
├── core/                          # LLM 核心库 (STATIC)
│   ├── message/                   # 聊天消息层次结构
│   │   ├── message (基类)         # role + content
│   │   ├── user_message           # attachments + tool_call_results
│   │   └── assistant_message      # tool_calls
│   ├── context/                   # 对话上下文管理
│   ├── llm_api/                   # LLM Provider 抽象
│   │   ├── llm_api.ixx            # Provider 接口
│   │   ├── api_registry.ixx       # 线程安全的 Provider 工厂注册表
│   │   └── chat_completion_api/   # OpenAI 兼容实现 (ai-sdk-cpp)
│   ├── agent/                     # 高层 Agent（对话循环 + tool 处理）
│   │   └── generate_text(ctx)     # 自动处理多步 tool calls
│   └── app_config/                # 配置管理（JSON 文件）
│
├── mcp_client/                    # MCP 客户端库 (STATIC)
│   ├── jsonrpc/                   # JSON-RPC 2.0 消息类型
│   ├── transport/                 # 传输层抽象
│   │   └── stdio_transport/       # Stdio 子进程传输
│   ├── mcp_client/                # 单个 MCP 服务器客户端
│   │   ├── initialize / list_tools / call_tool
│   │   └── list_resources / read_resource
│   └── mcp_manager/               # 多服务器编排
│       ├── load(config)           # 从 mcpServers JSON 加载
│       ├── get_tools()            # 聚合所有工具（带 server 前缀）
│       └── call_tool("server/tool", args)
│
├── external/                      # 嵌入式第三方依赖 (git submodules)
│   ├── ai-sdk-cpp/                # ClickHouse AI SDK (C++20)
│   └── catch2/                    # Catch2 v3 (测试框架)
│
├── app/                           # CLI 可执行文件（装配 + REPL）
├── tests/                         # Catch2 单元测试
└── config.json                    # 配置文件（示例，已加入 .gitignore）
```

## 环境要求

- **CMake** 3.28+
- **C++23** 编译器 (MSVC 19.40+ / GCC 13+)
- **OpenSSL** 开发头文件
- **npx**（运行 MCP 服务器用，可选）

## 构建

```bash
# 带 submodules 克隆
git clone --recursive https://github.com/1ml1ml/cppagent.git
cd cppagent

# 或已克隆后初始化 submodules
git submodule update --init --recursive

# 构建
cmake -B build -S .
cmake --build build --config Release
```

## 配置

在 `cppagent` 根目录创建 `config.json`：

```json
{
  "model": {
    "api": "openai",
    "model": "moonshot-v1-128k",
    "base_url": "https://api.moonshot.cn",
    "api_key": "sk-你的-api-key"
  },
  "mcpServers": {
    "filesystem": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-filesystem", "D:/"]
    }
  }
}
```

> `config.json` 已加入 `.gitignore`，不会误提交到仓库。

## 运行

```bash
# Windows
build\app\Release\cppagent.exe

# Linux/WSL
./build/app/cppagent
```

控制台以 REPL 方式读取输入，输入提示词后按回车发送。
输入 `/exit` 退出。

## MCP 使用

```cpp
import mcp_manager;

// 加载配置
mcp_manager mcp;
mcp.load(config);

// 获取聚合后的工具（自动加上 server/ 前缀）
auto tools = mcp.get_tools();
// [{"type":"function","function":{"name":"filesystem/read_file",...}}]

// 通过 manager 调用 tool（自动路由到对应 server）
auto result = mcp.call_tool("filesystem/read_file", {{"path", "D:/test.txt"}});
```

## Agent 使用

`agent` 类封装了完整的对话循环和 tool-call 处理：

```cpp
#include <nlohmann/json.hpp>
import agent;
import api_registry;
import chat_completion_api;
import context;
import message;
import mcp_manager;

// 1. 注册 Provider 工厂
api_registry::instance().register_factory(
    "openai", std::make_shared<chat_completion_api_factory>());

// 2. 创建并配置 Agent
auto chat_agent = std::make_shared<agent>();

nlohmann::json model_config;
model_config["api"]      = "openai";
model_config["model"]    = "moonshot-v1-128k";
model_config["base_url"] = "https://api.moonshot.cn";
model_config["api_key"]  = "sk-xxx";
chat_agent->set_model_config(model_config);

auto mcp = std::make_shared<mcp_manager>();
mcp->load(mcp_config_json);
chat_agent->set_mcp_manager(mcp);

// 3. 聊天
auto ctx = std::make_shared<context>();
ctx->set_instructions("You are a helpful assistant");
ctx->append(std::make_shared<user_message>("List files in D:/"));

chat_agent->generate_text(ctx);  // 自动处理 tool calls
std::cout << ctx->messages_ref().back()->get_content() << "\n";
```

## 核心设计决策

- **snake_case** — 所有标识符（类名、函数名、变量名）统一使用 snake_case
- **PIMPL** — 所有公共类使用 PIMPL 模式隐藏实现细节
- **Allman 大括号风格** — 2 空格缩进
- **统一初始化 `{}`** — 禁止使用 `=` 或 `()` 初始化
- **消息层次结构** — `message` 基类 + `user_message` / `assistant_message` 子类，visitor 模式处理 Provider 特定序列化
- **agent 类** — 封装完整的 generate + tool-call + result 循环
- **ai-sdk-cpp** — 处理 HTTP、JSON schema、重试逻辑

## 已知问题 / 限制

- **Windows-only app：** `main.cpp` 使用 `SetConsoleCP` / `SetConsoleOutputCP`（Windows API）实现 UTF-8 控制台 I/O。Linux/WSL 构建需要替换这些调用。
- **无 vcpkg：** 所有第三方依赖都嵌入在 `external/` 下（ai-sdk-cpp 自带 nlohmann_json、cpp-httplib 等）。

## License

MIT
