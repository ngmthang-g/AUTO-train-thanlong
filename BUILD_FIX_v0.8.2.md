# Build fix v0.8.2

GitHub Actions/Clang with `-Wall -Wextra -Werror` reported:

`unused variable 'EntryKey' [-Werror,-Wunused-const-variable]`

`off::EntryKey` was dead code: it was declared once and had no references anywhere in the source tree.
It has been removed rather than suppressed with `[[maybe_unused]]`.

No runtime/game logic was changed by this build fix.
