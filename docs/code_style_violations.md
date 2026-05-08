# 代码规范审查报告（修订版）

## 大哥确认的规范

1. **不要有移动构造/赋值** — PIMPL 类不需要
2. **`{` `}` 单独占一行** — Allman 风格
3. **缩进 2 空格** — 不用 tab

---

## 审查文件

- `lib/src/provider/llm_provider.ixx`
- `lib/src/provider/openai_provider/openai_provider.ixx`
- `lib/src/provider/openai_provider/openai_provider.cpp`
- `lib/src/provider/provider_registry.ixx`
- `lib/src/provider/provider_registry.cpp`
- `lib/src/context/context.ixx`
- `lib/src/context/context.cpp`
- `app/main.cpp`

---

## 已修复的问题

### ✅ 括号风格 — Allman

修复前（K&R）：
```cpp
export struct usage_info {
  int prompt_tokens{0};
};
```

修复后（Allman）：
```cpp
export struct usage_info
{
  int prompt_tokens{0};
};
```

### ✅ 缩进 — 2 空格

修复前（tab）：
```cpp
	int prompt_tokens{0};
```

修复后（2 空格）：
```cpp
  int prompt_tokens{0};
```

### ✅ 初始化 — 使用 `{}`

```cpp
int prompt_tokens{0};  // ✅
```

---

## 未修复的（不需要修复）

| 问题 | 原因 |
|------|------|
| PIMPL 缺移动构造/赋值 | 大哥明确说不需要 |

---

## 结论

所有 provider 层和 context 层的代码已统一为：
- Allman 括号风格
- 2 空格缩进
- `{}` 统一初始化
