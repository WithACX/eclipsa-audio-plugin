---
id: empirical-verification-refutes-theoretical-bugs
trigger: "when a theoretical code-safety concern is refuted by testing against the actual compiled library"
confidence: 0.7
domain: testing
scope: project
date: 2026-07-28
---
# Empirical Verification Can Refute Theoretical Safety Concerns

## Action
When a verifier reports a CRITICAL use-after-free / undefined-behavior / contract-violation concern (especially involving third-party C libraries like GPAC), test the exact pattern against the actual compiled binary to determine if the theoretical concern manifests in practice. A passing test on the real library (not mocked) is evidence that the concern may be non-critical code-hygiene violation rather than a crash/UB risk.

## Evidence
- PR #17 security review flagged a CRITICAL issue: `getMediaDurationSeconds()` calls raw GPAC ISOBMFF APIs after GPAC global teardown via `gf_sys_close()`, violating GPAC's own documented contract that `gf_sys_init()` MUST be called before any libgpac function
- Theoretical concern: use-after-teardown of GPAC global state, potential crash/UB/memory corruption on every video export
- Empirical verification: the same pattern (raw `gf_isom_*` calls post-`gf_sys_close()`) already exists on the base branch in `FileOutputTestUtils.h:getMP4DurationSeconds()`, called post-mux in tests
- Running the full test suite against the actual vendored `libgpac.dylib`: 35/37 tests pass, including the test that explicitly exercises the post-teardown GPAC call; no crashes, correct duration values returned
- Verdict: the finding SURVIVES as a code-hygiene / spec-contract violation, but is REFUTED as a CRITICAL crash/UB bug; the real GPAC library appears to have internal refcounting/idempotency that makes the pattern safe in practice

## How to apply
1. For concerns about library contract violations (init/close, state management, documented preconditions), prefer testing the actual pattern against the real compiled library over theoretical code-safety reasoning
2. If the finding already exists and works in the codebase (pre-dating the PR), test it; if tests pass, downgrade severity from CRITICAL to code-hygiene
3. Document the refutation outcome and the test evidence (exit code, assertion results, no crashes) so the finding's reclassification is justified
4. Distinguish between "the pattern violates the documented contract" (true, code-hygiene issue) and "the pattern causes crashes/UB in practice" (false if empirically verified)

## Related
- [[adversarial_finding_verification]] — adversarial verifiers should test empirically when possible
- [[regression_test_capture_from_review]] — leveraging existing test infrastructure as evidence
