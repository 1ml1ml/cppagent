# cppagent

C++23 LLM Agent Framework with MCP (Model Context Protocol) support.

## Features

- **C++23 Modules** — Modern modular C++ with `.ixx` interface units
- **Multi-Provider LLM** — Pluggable provider registry (OpenAI-compatible APIs)
- **MCP Client** — Full Model Context Protocol client implementation
  - JSON-RPC 2.0 over stdio transport
  - Multi-server management with `mcp_manager`
  - Tool discovery and invocation with server namespacing (`server/tool`)
  - Resource listing and reading
  - Roots support with change notifications
- **Cross-Platform** — Windows (MSVC) + Linux (WSL/GCC)
- **CMake 3.28+** with vcpkg integration

## Architecture

```
cppagent/
├── core/                          # LLM core library (STATIC)
│   ├── message/                   # Chat message types (system/user/assistant/tool)
│   ├── context/                   # Conversation context management
│   └── provider/                  # LLM provider abstraction
│       ├── llm_provider.ixx       # Provider interface
│       ├── provider_registry.ixx  # Thread-safe provider factory registry
│       └── openai_provider/       # OpenAI-compatible API implementation
│
├── mcp_client/                    # MCP client library (STATIC)
│   ├── jsonrpc/                   # JSON-RPC 2.0 message types
│   ├── transport/                 # Transport abstraction
│   │   └── stdio_transport/       # Stdio subprocess transport (Windows/Linux)
│   ├── mcp_client/                # Single MCP server client
│   │   ├── initialize / list_tools / call_tool
│   │   ├── list_resources / read_resource
│   │   └── roots support (list + change notifications)
│   └── mcp_manager/               # Multi-server orchestration
│       ├── load(config)           # Load from mcpServers JSON
│       ├── get_tools_schema()     # Aggregate tools with server prefix
│       └── call_tool("server/tool", args)
│
├── app/                           # CLI executable
└── tests/                         # Catch2 unit tests
```

## Prerequisites

- **CMake** 3.28+
- **C++23** compiler (MSVC 19.40+ / GCC 13+ / Clang 17+)
- **vcpkg** with `nlohmann-json` and `liboai` installed

## Build

```bash
# Windows (Visual Studio + vcpkg)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release

# Linux/WSL (vcpkg)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

## Run Tests

```bash
cd build
ctest -C Release
```

## MCP Configuration

Create a config file and load it into `mcp_manager`:

```json
{
  "mcpServers": {
    "filesystem": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-filesystem", "/home/user/project"]
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
#include <mcp_manager.hpp>

// Load config
auto config = nlohmann::json::parse(std::ifstream("mcp_config.json"));

mcp_manager manager;
manager.load(config);

// Get aggregated tools (prefixed as "server/tool")
auto tools = manager.get_tools_schema();
// [{"type":"function","function":{"name":"filesystem/read_file",...}},
//  {"type":"function","function":{"name":"github/create_issue",...}}]

// Model returns tool_calls with "filesystem/read_file"
auto result = manager.call_tool("filesystem/read_file", {{"path", "/tmp/test.txt"}});
```

## LLM Provider Usage

```cpp
#include <provider_registry.hpp>
#include <openai_provider.hpp>

// Register
provider_registry::instance().register_factory("openai", std::make_shared<openai_factory>());

// Create and configure
auto provider = provider_registry::instance().create("openai");
provider->set_config(R"({"api_key":"sk-xxx","model":"gpt-4o"})"_json);

// Generate
auto ctx = context::from_messages({{"user", "Hello!"}});
auto result = provider->generate(ctx);
std::cout << result.content << "\n";
```

## Environment Variables

```bash
# Windows
set OPENAI_API_KEY=sk-xxxxx

# Linux/Mac
export OPENAI_API_KEY=sk-xxxxx
```

## Key Design Decisions

- **snake_case** for all identifiers (classes, functions, variables)
- **PIMPL** pattern for all public classes
- **Allman brace style** with 2-space indentation
- **Unified initialization `{}`** throughout
- **No move constructors** on PIMPL classes

## License

MIT
