# Agent Context: Code Style

Human source: `docs/zh/code-style.md`. Use this file as the active formatter,
comment, and naming contract for Helicon-owned code.

## Purpose

This doc gives AI a reusable baseline for large-scale formatting, naming, and
comment cleanup so the contract does not need to be re-explained every time.

## Current Reference

- `include/utils.h` is the current reference header.
- New file banners, section-comment style, and bilingual declaration comments
  should align with `include/utils.h`.
- Helicon-owned helper code should prefer snake_case when there is no stronger
  compatibility constraint.

## File Banner Contract

- Use the banner format shown in `include/utils.h`.
- Keep `@file`, `@brief`, `@brief.zh`, `@project`, `@author`, and `@date`.
- Add `@note.en` / `@note.zh` when a file needs to explain AI-facing versus
  human-facing comment intent.

## Comment Contract

- English comments optimize for AI retrieval, reasoning, and code context.
- Chinese comments optimize for fast human understanding.
- Public declarations should prefer Doxygen-style bilingual comments.
- Section headers should prefer this shape:

```cpp
//-----------------------------------------------------------------------------
// English Section Title
// Chinese Section Title
//-----------------------------------------------------------------------------
```

- Avoid low-signal comments that only restate obvious syntax.

## Naming Contract

For Helicon-owned code that adopts the repo style:

- free functions: snake_case
- helper methods: snake_case
- parameters: snake_case
- member variables: snake_case

Do not automatically rename these unless explicitly requested:

- existing type names
- external API identifiers
- macros
- include guards
- fields or struct names constrained by shader, ABI, spec, or upstream layout

## Compatibility-Header Rule

For shared compatibility headers such as `include/rhiHLSL.h`, normalize:

- file banner
- section comments
- bilingual explanations
- safe local layout

Do not automatically rename:

- upstream mirror struct names
- spec-constrained field names
- identifiers that would change shader layout or external compatibility

## Large-Scale Formatting Rules

- Work file-by-file or module-by-module instead of renaming the whole repo at once.
- Pause on compatibility layers, upstream mirrors, or shader-shared layouts and check constraints first.
- Unless the user explicitly asks, do not turn formatting work into semantic refactoring.
- If the style contract changes, update Chinese docs first and then sync English agent docs.
