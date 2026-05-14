# cppagent

C++23 LLM Agent Framework with MCP (Model Context Protocol) support.

## Features

- **C++23 Modules** — Modern modular C++ with `.ixx` interface units
- **Multi-Provider LLM** — Pluggable provider registry via [ai-sdk-cpp](https://github.com/ClickHouse/ai-sdk-cpp)
  - OpenAI / Anthropic / OpenAI-compatible APIs (OpenRouter, Moonshot, Kimi, etc.)
  - Automatic multi-step tool calling (framework handles tool → result → reply loop)
- **MCP Client** — Full Model Context Protocol client implementation
  - JSON-RPC 2.0 over stdio transport
  - Multi-server management with `mcp_manager`
  - Tool discovery and invocation with server namespacing (`server/tool`)
  - Resource listing and reading
- **CMake 3.28+**
- **Windows-focused** — `app` uses Windows console APIs; core libraries compile on Linux/WSL

## Architecture

```
cppagent/
├── core/                          # LLM core library (STATIC)
│   ├── message/                   # Chat message hierarchy
│   │   ├── message (base)         # role + content
│   │   ├── user_message           # attachments + tool_call_results
│   │   └── assistant_message      # tool_calls
│   ├── context/                   # Conversation context management
│   └── llm_api/                   # LLM provider abstraction
│       ├── llm_api.ixx            # Provider interface
│       ├── api_registry.ixx       # Thread-safe provider factory registry
│       └── chat_completion_api/   # OpenAI-compatible implementation (ai-sdk-cpp)
│
├── mcp_client/                    # MCP client library (STATIC)
│   ├── jsonrpc/                   # JSON-RPC 2.0 message types
│   ├── transport/                 # Transport abstraction
│   │   └── stdio_transport/       # Stdio subprocess transport
│   ├── mcp_client/                # Single MCP server client
│   │   ├── initialize / list_tools / call_tool
│   │   └── list_resources / read_resource
│   └── mcp_manager/               # Multi-server orchestration
│       ├── load(config)           # Load from mcpServers JSON
│       ├── get_tools_schema()     # Aggregate tools with server prefix
│       └── call_tool("server/tool", args)
│
├── external/                      # Embedded third-party dependencies (git submodules)
│   ├── ai-sdk-cpp/                # ClickHouse AI SDK (C++20)
│   └── catch2/                    # Catch2 v3 (tests disabled)
│
├── agent/                         # High-level agent (conversation loop + tool handling)
│   └── generate_text(ctx)         # Auto-handles multi-step tool calls
│
├── app/                           # CLI executable (assembly + REPL)
└── tests/                         # Catch2 unit tests (disabled — see Known Issues)
```

## Prerequisites

- **CMake** 3.28+
- **C++23** compiler (MSVC 19.40+ / GCC 13+)
- **OpenSSL** development headers
- **npx** (for MCP servers, optional)

## Build

```bash
# Clone with submodules
git clone --recursive https://github.com/1ml1ml/cppagent.git
cd cppagent

# Or if already cloned, init submodules
git submodule update --init --recursive

# Build
cmake -B build -S .
cmake --build build --config Release
```

> **Note:** Tests are currently disabled due to MSVC C++20 Modules + Catch2 v3 incompatibility. Run `app/cppagent` directly for validation.

## Run

```bash
# Windows — API key is loaded from D:\Sources\cppagent\api_key.txt
build\app\Release\cppagent.exe

# Linux/WSL
./build/app/cppagent
```

Console input is read in a REPL loop. Type your prompt and press Enter.
Type `/exit` to quit.

## MCP Configuration

MCP server config is hardcoded in `app/main.cpp` as a JSON string. Edit `load_mcp_config()` in `app/main.cpp` to customize:

```cpp
constexpr auto mcp_config_json = R"({
  "mcpServers": {
    "filesystem": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-filesystem", "C:/Users/xxx/Documents"]
    }
  }
})";
```

```cpp
import mcp_manager;

// Load config (hardcoded or from file)
mcp_manager mcp;
mcp.load(config);

// Get aggregated tools (prefixed as "server/tool")
auto tools = mcp.get_tools_schema();
// [{"type":"function","function":{"name":"filesystem/read_file",...}}]

// Call tool via manager (handles routing to correct server)
auto result = mcp.call_tool("filesystem/read_file", {{"path", "/tmp/test.txt"}});
```

## Agent Usage

The `agent` class encapsulates the conversation loop and tool-call handling:

```cpp
#include <nlohmann/json.hpp>
import agent;
import api_registry;
import chat_completion_api;
import context;
import message;
import mcp_manager;

// 1. Register provider factory
api_registry::instance().register_factory(
    "openai", std::make_shared<chat_completion_api_factory>());

// 2. Create and configure agent
auto chat_agent = std::make_shared<agent>();

nlohmann::json model_config;
model_config["api"]     = "openai";
model_config["model"]   = "moonshot-v1-128k";
model_config["base_url"]= "https://api.moonshot.cn";
model_config["api_key"] = "sk-xxx";
chat_agent->set_model_config(model_config);

auto mcp = std::make_shared<mcp_manager>();
mcp->load(mcp_config_json);
chat_agent->set_mcp_manager(mcp);

// 3. Chat
auto ctx = std::make_shared<context>();
ctx->set_instructions("You are a helpful assistant");
ctx->append(std::make_shared<user_message>("List files in D:/"));

chat_agent->generate_text(ctx);  // auto-handles tool calls
std::cout << ctx->messages_ref().back()->get_content() << "\n";
```

## Key Design Decisions

- **snake_case** for all identifiers (classes, functions, variables)
- **PIMPL** pattern for all public classes
- **Allman brace style** with 2-space indentation
- **Unified initialization `{}`** throughout
- **Message hierarchy** — `message` base class with `user_message` / `assistant_message` subclasses; visitor pattern for provider-specific serialization
- **agent** class wraps the full generate + tool-call + result loop
- **ai-sdk-cpp** handles HTTP, JSON schema, retry logic

## Known Issues / Limitations

- **Tests disabled:** Catch2 v3 + MSVC C++20 Modules = fatal compiler errors (C2572/C7571). Waiting for upstream fix or test framework replacement.
- **Windows-only app:** `main.cpp` uses `SetConsoleCP` / `SetConsoleOutputCP` (Windows API) for UTF-8 console I/O. Linux/WSL build requires replacing these calls.
- **No vcpkg:** All third-party dependencies are embedded under `external/` (ai-sdk-cpp bundles its own nlohmann_json, cpp-httplib, etc.).

## License

MIT
