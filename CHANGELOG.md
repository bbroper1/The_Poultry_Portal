# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Semantic Versioning](https://semver.org/).

---
[2.0.0] — 2026‑02‑15
Added
- Completely redesigned OLED UI
- Two‑screen layout (Main + Info)
- Clean text‑only interface
- Battery icon retained, all other indicators converted to text
- New two‑row Open/Close layout to prevent overflow
- Opening/Closing animation screen
- New DisplayTask architecture
- Auto‑switching screens
- Motion override screen
- Centralized rendering helpers
- Improved system boot screen
- Version now displayed on main screen
- New v2.0.0 version tag in Config
Improved
- Major cleanup of DisplayTask.cpp
- Removed emoji (OLED‑unsafe)
- Removed oversized icon bitmaps
- Reduced file size and complexity
- Improved readability and maintainability
- More consistent spacing and alignment across screens
- Better handling of door motion states
- More robust time extraction for Open/Close events
- Clearer separation between main and info screens
- Reduced RAM usage by removing unused graphics assets
Fixed
- Text overflow on main screen
- Incorrect rendering of UTF‑8 characters on OLED
- Occasional clipping during OPENING/CLOSING states
- Boot screen timing inconsistencies

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
