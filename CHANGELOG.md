# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Semantic Versioning](https://semver.org/).

---

## [1.0.0] — 2026-01-15
### Added
- Initial public release of PoultryPortal firmware.
- Complete modular refactor:
  - Display module
  - Motor control module
  - Telegram router and handlers
  - Config subsystem
- GitHub Actions CI workflow for automatic PlatformIO builds.
- MIT License added to repository.
- README badges for build status, license, version, and PlatformIO.
- `.gitignore` optimized for PlatformIO and macOS.

### Improved
- Project structure reorganized for clarity and maintainability.
- Reduced global variable usage; improved module encapsulation.
- More consistent naming and file organization across the project.

### Fixed
- Various compile and linkage issues discovered during refactor.
- Cleaned up unused code and legacy definitions.

---

## [Unreleased]
### Planned
- Add OTA update support.
- Add configuration menu on OLED.
- Add Telegram-based configuration for motor timeout and pinch threshold.
- Add automatic timezone detection based on location.
