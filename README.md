# cppagent

C++23 LLM Agent Framework with MCP (Model Context Protocol) support.

## Features

- **C++23 Modules** — Modern modular C++ with `.ixx` interface units
- **Multi-Provider LLM** — Pluggable provider registry via [ai-sdk-cpp](https://github.com/ClickHouse/ai-sdk-cpp)
  - OpenAI / Anthropic / OpenAI-compatible APIs (OpenRouter, Moonshot, etc.)
  - Automatic multi-step tool calling (framework handles tool → result → reply loop)
  - Streaming support
- **MCP Client** — Full Model Context Protocol client implementation
  - JSON-RPC 2.0 over stdio transport
  - Multi-server management with `mcp_manager`
  - Tool discovery and invocation with server namespacing (`server/tool`)
  - Resource listing and reading
- **Cross-Platform** — Windows (MSVC) + Linux (WSL/GCC)
- **CMake 3.28+** with vcpkg integration

## Architecture

```
cppagent/
├── core/                          # LLM core library (STATIC)
│   ├── message/                   # Chat message (role + type + content)
│   │   ├── role: user / assistant
│   │   └── type: text / tool_call / tool_result
│   ├── context/                   # Conversation context management
│   └── provider/                  # LLM provider abstraction
│       ├── llm_provider.ixx       # Provider interface + model_response
│       ├── provider_registry.ixx  # Thread-safe provider factory registry
│       └── openai_provider/       # OpenAI-compatible implementation (ai-sdk-cpp)
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
├── external/                      # Embedded third-party dependencies
│   └── ai-sdk-cpp/                # ClickHouse AI SDK (C++20)
│
├── app/                           # CLI executable
└── tests/                         # Catch2 unit tests (disabled — see #7)
```

## Prerequisites

- **CMake** 3.28+
- **C++23** compiler (MSVC 19.40+ / GCC 13+)
- **vcpkg** with `nlohmann-json`
- **OpenSSL** development headers (for ai-sdk-cpp HTTPS)

## Build

```bash
# Windows (Visual Studio + vcpkg)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release

# Linux/WSL (vcpkg)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

> **Note:** Tests are currently disabled due to MSVC C++20 Modules + Catch2 v3 incompatibility. Run `app/cppagent` directly for validation.

## Run

```bash
# Windows
set OPENAI_API_KEY=sk-xxxxx
build\app\Release\cppagent.exe

# Linux/WSL
export OPENAI_API_KEY=sk-xxxxx
./build/app/cppagent
```

Console input is read in a REPL loop. Type your prompt and press Enter.

## MCP Configuration

Create `mcp_config.json`:

```json
{
  "mcpServers": {
    "filesystem": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-filesystem", "C:/Users/xxx/Documents"]
    },
    "github": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-github"],
      "env": {
        "GITHUB_PERSONAL_ACCESS_TOKEN": "ghp_xxx"
      }
    }
  }
}
```

```cpp
#include <nlohmann/json.hpp>
import mcp_manager;

// Load config
auto config = nlohmann::json::parse(std::ifstream("mcp_config.json"));

mcp_manager mcp;
mcp.load(config);

// Get aggregated tools (prefixed as "server/tool")
auto tools = mcp.get_tools_schema();
// [{"type":"function","function":{"name":"filesystem/read_file",...}},
//  {"type":"function","function":{"name":"github/create_issue",...}}]

// Call tool via manager (handles routing to correct server)
auto result = mcp.call_tool("filesystem/read_file", {{"path", "/tmp/test.txt"}});
```

## LLM Provider Usage

```cpp
#include <nlohmann/json.hpp>
import context;
import message;
import llm_provider;
import openai_provider;
import provider_registry;
import mcp_manager;

// Register provider
provider_registry::instance().register_factory(
    "openai", std::make_shared<openai_provider_factory>());

// Create and configure
auto provider = provider_registry::instance().create("openai");
nlohmann::json cfg;
cfg["model"] = "moonshot-v1-8k";
cfg["base_url"] = "https://api.moonshot.cn";  // no /v1 suffix
cfg["api_key"] = "sk-xxx";
provider->set_config(cfg);

// Setup context + MCP
auto ctx = std::make_shared<context>();
ctx->set_instructions("You are a helpful assistant");
ctx->append(std::make_shared<message>(message::role::user, message::type::text, "Hello!"));

mcp_manager mcp;
mcp.load(config);

// Generate — tool calling is handled automatically by ai-sdk-cpp
auto resp = provider->generate(ctx, mcp.get_tools());
std::cout << resp->message->get_content() << "\n";
```

## Key Design Decisions

- **snake_case** for all identifiers (classes, functions, variables)
- **PIMPL** pattern for all public classes
- **Allman brace style** with 2-space indentation
- **Unified initialization `{}`** throughout
- **Message type enum** (`text`/`tool_call`/`tool_result`) separates content semantics from role
- **ai-sdk-cpp** handles HTTP, JSON schema, retry, streaming, and multi-step tool loops

## Known Issues

- **Tests disabled:** Catch2 v3 + MSVC C++20 Modules = fatal compiler errors (C2572/C7571). Waiting for upstream fix or test framework replacement.
- **Windows console UTF-8:** `SetConsoleCP(CP_UTF8)` is set in `main()`, but legacy terminals may still need `chcp 65001`.

## License

MIT
