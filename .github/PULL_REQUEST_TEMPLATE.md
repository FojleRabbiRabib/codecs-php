| Question | Answer |
| --- | --- |
| Target branch | `develop` |
| Type of change | feature / bugfix / build / docs / refactor |
| Formats affected | AVIF / JPEG XL / PNG / Core API / Build |
| License | Apache-2.0 |

### Summary & Motivation
<!-- Provide a clear, concise explanation of the change and the problem it solves. -->

### Technical Implementation
<!-- Outline key architectural decisions, C translation units touched, and memory/concurrency considerations. -->

### Verification & Testing
<!-- Describe how this was tested. Include command line output, fixtures, or test cases. -->
```bash
# Example verification runs:
php tools/test-phpt.php modules/codecs.so
php -n -d extension=modules/codecs.so tests/quality/fidelity.php
```

### Contributor Checklist
- [ ] Code follows project standards (`clang-format` for C, PSR-12 for PHP)
- [ ] Compiles with zero warnings under `-Wall -Wextra -Werror`
- [ ] Added or updated test cases in `tests/phpt/` or fidelity oracle
- [ ] All tests pass locally on PHP 8.3
- [ ] Symbol isolation verified (`nm -D` exports only `get_module`)
- [ ] Resource ceilings and memory limits respected
- [ ] Updated `CHANGELOG.md` under `[Unreleased]` for user-facing changes
- [ ] Public documentation updated if API or options changed
