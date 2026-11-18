---
status: accepted
---

# Two keymap families share a common core on one branch

This repository maintains two keymap families side by side for the long term: `hydeik` on upstream QMK and `hydeik_vial` on vial-qmk. They had drifted apart through copy-and-paste, and which firmware source a build target used was implied by its keymap name. We keep both families on one branch, each in its own `users/` directory, and put everything that does not depend on the keymap family in a third directory, the common core (`users/hydeik_common/`), which each family includes by full path. Each family is built against exactly one firmware source, declared and pinned to a commit in `firmware_sources.json`.

The common core follows three rules:

1. It holds only what does not depend on the keymap family. What both families use is deliberately kept identical; what one family alone needs stays with that family.
2. It never branches on the keymap family or the firmware source (no `#ifdef VIAL_ENABLE`). Differences are passed in by the family as parameters.
3. Dependencies point one way, from a keymap family to the common core. The common core never refers to a family, and the families never refer to each other.

## Considered Options

- **One branch per keymap family.** Rejected: every change to something both families share would need cherry-picking across branches, which recreates the drift, and one `qmk.json` and one CI workflow already serve both firmware sources from a single branch.
- **One `users/` directory for both families, switching on `VIAL_ENABLE`.** Rejected: the families differ in most of their code (tap-hold tuning versus custom one-shot mods), and a default Vial build defines the tap-hold callbacks itself, so a merged directory would be conditionals throughout.
- **Unpinned firmware sources, selected by a `_vial` keymap-name suffix.** Rejected: builds broke from upstream changes alone, and any keymap name without the suffix silently went to upstream QMK.

## Consequences

- `hydeik_vial` keeps Vial's GUI-driven tap-hold settings and does not reuse `hydeik`'s tap-hold tuning. Sharing it would mean turning off Vial features.
- Not every keymap family exists on every keyboard. The build targets in `qmk.json` are the complete list, and every keymap directory on `main` must be one of them.
- A firmware source only changes when its pin is moved on purpose, on a branch, with CI passing before it reaches `main`.
- Local builds go through `make`, which selects the pinned firmware checkout by keymap family. `qmk compile` with a single `qmk_home` is not supported.

The detailed decisions behind this (file names, macro names, which settings and callbacks are shared) are recorded in the tickets of issue #1.
