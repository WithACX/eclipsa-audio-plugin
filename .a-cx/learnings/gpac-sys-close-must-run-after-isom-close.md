---
id: gpac-sys-close-must-run-after-isom-close
trigger: "when adding gf_sys_init/gf_sys_close around GPAC calls that also open a GF_ISOFile via RAII"
confidence: 0.7
domain: code-style
scope: project
date: 2026-07-30
---
# gf_sys_close() must run after the RAII-owned GF_ISOFile is closed, not before

## Action
When bracketing a function's GPAC calls in `gf_sys_init()`/`gf_sys_close()`
(required per GPAC's documented lifecycle -- see
[[gpac-lifecycle-review-finding]]), and the function also opens a
`GF_ISOFile` via a `std::unique_ptr<GF_ISOFile, decltype(&gf_isom_close)>`,
do not simply add a bare `gf_sys_close()` call before each `return`. The
`unique_ptr`'s destructor (which calls `gf_isom_close`) only runs when the
enclosing scope exits, which happens *after* any statement preceding the
`return` -- so a `gf_sys_close(); return x;` pair closes GPAC's global state
before the isom file is closed, violating the very lifecycle rule being
fixed. Wrap the `unique_ptr`'s scope in an explicit nested `{ }` block (or
call `.reset()` explicitly) so `gf_isom_close()` completes before the
trailing `gf_sys_close()` runs.

## Evidence
- Found while fixing `IAMFExportHelper::getMediaDurationSeconds` in
  `common/processors/file_output/iamf_export_utils/IAMFExportUtil.cpp` for
  a MEDIUM review finding on PR #17 (missing gf_sys_init/close bracket). The
  first draft called `gf_sys_close()` immediately before each `return`,
  which would close GPAC before the RAII `unique_ptr` running
  `gf_isom_close` on scope exit -- caught before committing by tracing the
  actual destructor-vs-statement ordering, not by a test failure.

## Related
- [[lfs-pointer-file-changes-in-worktree]]
