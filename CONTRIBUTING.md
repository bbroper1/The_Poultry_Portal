Contributing to PoultryPortal
Thank you for your interest in contributing to PoultryPortal.
This project follows a clean, modular structure and aims for professional‑grade firmware quality.
The guidelines below help ensure consistency, reliability, and maintainability.

🛠 Development Workflow
1. Fork and clone the repository
git clone https://github.com/<your-username>/The_Poultry_Portal.git


2. Create a feature branch
git checkout -b feature/my-new-feature


3. Make your changes
Follow the project’s coding style and module structure:
- Keep logic modular (Display, Motor, Config, Telegram, etc.)
- Avoid introducing new global variables unless absolutely necessary
- Prefer small, focused commits
4. Run a local build
pio run


5. Ensure CI passes
All pull requests must pass the GitHub Actions PlatformIO build.
6. Submit a Pull Request
Include:
- A clear description of the change
- Any relevant screenshots or logs
- A reference to an issue if applicable

🧹 Code Style Guidelines
- Use descriptive names for functions and variables
- Keep modules self‑contained
- Avoid blocking delays where possible
- Document non‑obvious logic with brief comments
- Maintain consistent formatting across files

🧪 Testing
Before submitting a PR:
- Verify the firmware compiles without warnings
- Test door movement logic if applicable
- Confirm display and Telegram flows behave as expected
- Validate configuration changes persist correctly

📁 Project Structure
/src        → Main firmware modules
/include    → Headers and shared interfaces
/lib        → External libraries (if needed)
/data       → SPIFFS or LittleFS assets



📝 Commit Messages
Use clear, concise commit messages:
- Add motor timeout safety check
- Fix Telegram /status formatting
- Refactor Display module for clarity
Avoid vague messages like “fix stuff”.

🧭 Reporting Issues
If you find a bug or have a feature request:
- Open an issue on GitHub
- Include steps to reproduce (if applicable)
- Attach logs or screenshots when helpful

❤️ Thank You
Your contributions help make PoultryPortal more reliable, maintainable, and useful for everyone.
