# Contributing to IoT HVAC Control System

Thank you for considering contributing to the IoT HVAC Control System! This document provides guidelines and instructions for contributing.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Commit Guidelines](#commit-guidelines)
- [Pull Request Process](#pull-request-process)
- [Testing](#testing)

## 🤝 Code of Conduct

This project follows a Code of Conduct to ensure a welcoming environment:
- Be respectful and inclusive
- Accept constructive criticism
- Focus on what's best for the community
- Show empathy towards others

## 💡 How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check existing issues to avoid duplicates.

**Good Bug Report Includes**:
- Clear, descriptive title
- Steps to reproduce the behavior
- Expected vs actual behavior
- Screenshots/logs if applicable
- Environment details (OS, hardware, versions)

### Suggesting Enhancements

Enhancement suggestions are welcome! Please provide:
- Clear use case description
- Why this enhancement would be useful
- Possible implementation approach
- Any alternatives you've considered

### Code Contributions

Areas where contributions are especially welcome:
- Support for additional AC brands (IR protocols)
- Mobile app development
- Database backend implementation
- Authentication/security features
- Unit tests and integration tests
- Documentation improvements
- Bug fixes

## 🔧 Development Setup

### Prerequisites
- Python 3.9+
- PlatformIO
- Git
- Hardware (for testing): Arduino R4, ESP8266, sensors

### Setup Steps

1. **Fork and Clone**
```bash
git clone https://github.com/YOUR_USERNAME/IOT-HVAC-System.git
cd IOT-HVAC-System
```

2. **Set Up Python Environment**
```bash
cd server
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install -r requirements.txt
pip install -r requirements-dev.txt  # For development tools
```

3. **Install PlatformIO**
```bash
pip install platformio
```

4. **Create Feature Branch**
```bash
git checkout -b feature/your-feature-name
```

## 📝 Coding Standards

### Python (Server)
- Follow PEP 8 style guide
- Use type hints where appropriate
- Maximum line length: 100 characters
- Use docstrings for functions and classes

**Example**:
```python
def calculate_schedule_status(start_time: str, end_time: str) -> bool:
    """
    Determine if current time falls within schedule.
    
    Args:
        start_time: Start time in HH:MM format
        end_time: End time in HH:MM format
    
    Returns:
        True if current time is within schedule, False otherwise
    """
    # Implementation...
```

### C++ (Firmware)
- Follow Arduino style conventions
- Use meaningful variable names
- Add comments for complex logic
- Maximum line length: 100 characters

**Example**:
```cpp
/**
 * @brief Update AC settings based on HVAC configuration
 * @param settings Current HVAC settings structure
 * @return true if update successful, false otherwise
 */
bool updateAC(const HVACSettings& settings) {
    // Implementation...
}
```

### HTML/CSS (Templates)
- Use semantic HTML5 elements
- Mobile-first responsive design
- Accessible (ARIA labels where needed)
- Consistent indentation (2 spaces)

## 📊 Commit Guidelines

### Commit Message Format
```
type(scope): brief description

Detailed explanation (optional)

Fixes #123
```

### Types
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

### Examples
```
feat(schedule): add weekly schedule support

Implemented weekly scheduling with day-of-week selection.
Users can now set different schedules for different days.

Fixes #45
```

```
fix(esp8266): resolve WiFi reconnection issue

ESP now automatically reconnects to WiFi if connection drops.
Added retry logic with exponential backoff.

Closes #67
```

## 🔄 Pull Request Process

### Before Submitting

1. **Test Your Changes**
   - Test on actual hardware if possible
   - Verify web interface works
   - Check for console errors
   - Run any existing tests

2. **Update Documentation**
   - Update README.md if needed
   - Add/update code comments
   - Update relevant .md files in docs/

3. **Check Code Quality**
   - No commented-out code
   - Remove debug print statements
   - Follow coding standards
   - Ensure code is well-documented

### Submission Steps

1. **Push to Your Fork**
```bash
git push origin feature/your-feature-name
```

2. **Create Pull Request**
   - Use a clear, descriptive title
   - Reference related issues
   - Describe what changed and why
   - Add screenshots for UI changes
   - List any breaking changes

3. **PR Template**
```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Tested on actual hardware
- [ ] Tested web interface
- [ ] No console errors

## Checklist
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] No commented-out code
- [ ] Commit messages follow conventions
```

### Review Process

- Maintainers will review your PR
- Address any requested changes
- Once approved, PR will be merged
- Thank you for your contribution! 🎉

## 🧪 Testing

### Manual Testing

**Hardware Tests**:
1. Flash firmware to both boards
2. Verify displays show data
3. Test rotary encoder navigation
4. Check sensor readings accuracy
5. Test IR LED transmission (phone camera)

**Web Interface Tests**:
1. Access dashboard and control panel
2. Test all control buttons
3. Verify real-time updates
4. Test schedule functionality
5. Check mobile responsiveness

**Integration Tests**:
1. Change setting on Arduino → Check web updates
2. Change setting on web → Check Arduino updates
3. Set schedule → Verify automatic control
4. Test overnight schedule (23:00-05:00)

### Automated Testing (Future)

We welcome contributions for:
- Unit tests for Python server
- Integration tests for API endpoints
- Hardware-in-the-loop tests
- CI/CD pipeline setup

## 📄 License

By contributing, you agree that your contributions will be licensed under the MIT License.

## 🙋 Questions?

- Open an issue for questions
- Tag with `question` label
- We're here to help!

---

Thank you for contributing to making IoT HVAC control better! 🌟
