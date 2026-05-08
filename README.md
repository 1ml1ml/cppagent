# cppagent

C++ AI agent framework with C++20 Modules.

## Project Structure

```
cppagent/
├── lib/                          # Static library (cppagent_lib)
│   ├── src/
│   │   ├── clients/              # Client interfaces & implementations
│   │   ├── context/              # Message context management
│   │   ├── message/              # Message types
│   │   └── models/               # Model registry & implementations
│   └── CMakeLists.txt
├── app/                          # Executable target
│   └── main.cpp
├── tests/                        # Unit tests (Catch2 v3)
│   ├── test_message.cpp
│   ├── test_context.cpp
│   ├── test_model_registry.cpp
│   ├── test_standard_model.cpp
│   └── test_openai_client.cpp
├── external/                     # Git submodules (e.g., catch2)
└── CMakeLists.txt
```

## Workflow Conventions

- **main branch**: All development happens here. No long-lived feature branches.
- **Tests live with code**: `tests/` is part of main, not a separate branch.
- **Commit format**: `type: description` (e.g., `feat:`, `fix:`, `test:`, `refactor:`)
- **CI/CD**: GitHub Actions runs build + tests on every push/PR.

## Development

### Prerequisites
- Visual Studio 2022 (C++20 Modules support)
- CMake 3.28+
- vcpkg (nlohmann_json, curl, etc.)

### Build
```powershell
cmake -B out/build/x64-Debug -S . -G "Visual Studio 17 2022" -A x64
cmake --build out/build/x64-Debug --config Debug
```

### Run Tests
```powershell
cd out/build/x64-Debug
ctest -C Debug
```

Or in Visual Studio: Test → Test Explorer → Run All.

## Dependencies

- nlohmann_json (vcpkg)
- liboai (custom, D:/third_party/liboai)
- Catch2 v3.14.0 (external/catch2)
