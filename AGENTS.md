# ASR5K Workspace Governance

This public repository follows the ASR5K workspace governance model.

Canonical governance source:

- https://github.com/linwuyen/ASR5K_AGENT

Access note:

- The canonical governance repository is private/internal.
- This public repository intentionally does not include the private `.agent` submodule.
- External users can clone, build, and inspect this repository without access to `ASR5K_AGENT`.
- No production firmware, build script, or hardware evidence depends on the private governance repository.

AI agent constraints:

- Default mode is read-only.
- Do not modify firmware source files unless explicitly authorized by the user.
- Do not edit `.c`, `.h`, `.syscfg`, `.cmd`, `.project`, `.cproject`, linker, build, flash, or generated files without explicit task scope.
- Do not run CCS, eclipsec, build, flash, git push, or git reset unless explicitly authorized.
- Before any edit, report branch, commit, dirty state, intended files, and risk.
