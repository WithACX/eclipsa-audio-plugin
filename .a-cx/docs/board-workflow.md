# Groove board workflow (A-CX internal)

Internal A-CX process doc. Lives under `.a-cx/` so the pre-push guard and the
external-fork branch topology keep it out of any upstream-bound diff. Never
move this out of `.a-cx/` on this fork.

How the org "Groove" board (project #1) is configured for the Eclipsa project.
The board tracks triaged work; the repo issue list is the full record. Issues
themselves live in `WithACX/eclipsa-audio-planning`.

## Status flow

The board uses a product-management triage gate on entry, then follows the
delivery pipeline. Custom rules deviate from the A-CX defaults where noted.

| Trigger (built-in workflow) | Sets Status | Notes |
|---|---|---|
| Item added to project | **User Comms (For Mikko)** | PM triage gate (custom; A-CX default is Todo). Mikko promotes to Todo once triaged. |
| (PM promotes, manual) | Todo | Now eligible for delivery. |
| Item picked by delivery | In Progress | Set by `/deliver-code` at pickup, not a built-in. |
| Pull request linked to issue | In Review | Switch to In Progress if draft PRs are linked early. |
| Code changes requested | In Progress | Review asked for changes; back to active work. |
| Code review approved | In Review | |
| Pull request merged | Done | |
| Item closed | Done | |
| Item reopened | **Backlog** | Custom (A-CX default is Todo); reopened work is re-prioritized. |

## Auto-add

Only triaged issues join the board. Auto-add is enabled on
`WithACX/eclipsa-audio-planning` with this filter:

```
is:issue label:"feature request",investigate,priority:0-blocker,priority:1-high,priority:2-minor,priority:3-deferred
```

An issue joins the board the moment it gets a priority label or a planned type
(`feature request` / `investigate`). Untriaged quick fixes stay as repo issues
and never clutter the board.

## Auto-archive

Closed issues are archived off the board once stale:

```
is:issue is:closed updated:<@today-2w
```

## Other workflows

- Auto-add sub-issues to project: on.

## Delivery implication

`/deliver-code` picks work from **Todo**. Freshly added issues sit in the
"User Comms (For Mikko)" gate until PM promotes them, and reopened issues land
in **Backlog**, so neither is treated as ready to build. Only Todo-or-beyond
items are worked by the pipeline.
