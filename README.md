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
- **OpenSSL** development headers
- **npx** (for MCP servers, optional)

## Build

```bash
cmake -B build -S .
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
cfg["model"] = "kimi-k2.6";
cfg["base_url"] = "https://api.moonshot.cn";  // no /v1 suffix
cfg["api_key"] = "sk-xxx";
provider->set_config(cfg);

// Setup context + MCP
auto ctx = std::make_shared<context>();
ctx->set_instructions("You are a helpful assistant");
ctx->append(std::make_shared<user_message>("Hello!"));

mcp_manager mcp;
mcp.load(config);

// Generate — tool calling is handled automatically by ai-sdk-cpp
auto resp = provider->generate_text(ctx, mcp.get_tools());
std::cout << resp->message->get_content() << "\n";
```

## Key Design Decisions

- **snake_case** for all identifiers (classes, functions, variables)
- **PIMPL** pattern for all public classes
- **Allman brace style** with 2-space indentation
- **Unified initialization `{}`** throughout
- **Message hierarchy** — `message` base class with `user_message` / `assistant_message` subclasses; visitor pattern for provider-specific serialization
- **ai-sdk-cpp** handles HTTP, JSON schema, retry, and multi-step tool loops

## Known Issues / Limitations

- **Tests disabled:** Catch2 v3 + MSVC C++20 Modules = fatal compiler errors (C2572/C7571). Waiting for upstream fix or test framework replacement.
- **Windows-only app:** `main.cpp` uses `SetConsoleCP` / `SetConsoleOutputCP` (Windows API) for UTF-8 console I/O. Linux/WSL build requires replacing these calls.
- **No vcpkg:** All third-party dependencies are embedded under `external/` (ai-sdk-cpp bundles its own nlohmann_json, cpp-httplib, etc.).

## License

MIT
