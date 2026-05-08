# cppagent

C++20 Modules + CMake project for LLM agent.

## 架构

```
lib/src/
  core/
    generation_result.ixx    # 生成结果结构（含 usage/token）
    config_validator.ixx     # 配置校验
  provider/
    llm_provider.ixx         # provider 接口（合并 model + client）
    provider_registry.ixx    # provider 注册表（线程安全）
    openai_provider/         # OpenAI 实现
      openai_provider.ixx
      openai_provider.cpp
  message/
  context/

app/
  main.cpp                   # CLI 入口

tests/
  test_*.cpp                 # 单元测试
```

## 构建

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

## 测试

```bash
cd build
ctest -C Release
```

## 使用

```cpp
// 注册 provider
provider_registry::instance().register_factory("openai", std::make_shared<openai_factory>());

// 创建并使用
auto provider = provider_registry::instance().create("openai");
provider->set_config(config);
auto result = provider->generate(ctx);
std::cout << result.message->get_content() << "\n";
```

## 设置 API Key

```bash
set CPPGENT_API_KEY=sk-xxxxx   # Windows
export CPPGENT_API_KEY=sk-xxxxx # Linux/Mac
```
