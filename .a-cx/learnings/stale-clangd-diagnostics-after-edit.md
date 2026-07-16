---
id: stale-clangd-diagnostics-after-edit
trigger: "when the editor/LSP surfaces new diagnostics (undeclared identifier, unknown type name, no member named X) immediately after editing a C++ header or source file in this repo"
confidence: 0.6
domain: process
scope: project
date: 2026-07-15
---
# clangd diagnostics can lag real edits by several tool calls in this repo

## Action
Do not treat live clangd/LSP diagnostics as ground truth right after editing a
C++ file in this codebase -- they can report "undeclared identifier" or
"unknown type name" for symbols that a real `cmake --build` compiles cleanly,
because clangd's index/compile-database view lags the actual file state for
several tool calls (likely due to the size of this project's compile
database and precompiled dependency tree). Before concluding an edit is
broken from diagnostics alone, run the actual build
(`cmake --build build --target <affected-target> -j8`) and trust that result
instead.

## Evidence
- Observed 3 times during the #41 delivery run (worktree
  `.a-cx/worktrees/41-export-fails-silently`): after adding the `ExportError`
  enum to `FileExport.h` and using it in `FileOutputProcessor.cpp`, clangd
  reported `kNoError`/`kPermissionDenied`/etc. as undeclared and
  `getExportError`/`setExportError` as missing members -- on code that was
  syntactically correct and that a real `cmake --build` compiled without
  error moments later. The same false pattern recurred after two subsequent,
  unrelated edits (`WarningBannerBase.h` extraction, then
  `ExportErrorBanner.h`), each time resolving to a real build success.
