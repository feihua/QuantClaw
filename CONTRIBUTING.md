# Contributing to QuantClaw

Thank you for your interest in contributing to QuantClaw! This document provides guidelines and instructions for contributing to the project.

## Code of Conduct

By participating in this project, you agree to abide by our [Code of Conduct](CODE_OF_CONDUCT.md).

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/your-username/quantclaw.git`
3. Create a feature branch: `git checkout -b feature/your-feature`
4. Make your changes
5. Commit your changes: `git commit -m "Add your feature"`
6. Push to the branch: `git push origin feature/your-feature`
7. Open a pull request

## Development Setup

### Prerequisites

- C++20 compiler (GCC 10+, Clang 12+, MSVC 19.29+)
- CMake 3.20+
- Git

### Building

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Testing

```bash
# Build with tests
cmake -DBUILD_TESTS=ON ..
make test
```

## Coding Guidelines

### C++ Style

- Use C++20 features where appropriate
- Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with the following modifications:
  - Use `snake_case` for function and variable names
  - Use `PascalCase` for class and struct names
  - Use `UPPER_SNAKE_CASE` for constants
  - Maximum line length: 120 characters

### Header Files

- Use `#pragma once` instead of include guards
- Include necessary headers only
- Keep header files as lightweight as possible

### Error Handling

- Use exceptions for exceptional circumstances
- Use `std::expected` (C++23) or custom result types for expected errors
- Log errors appropriately using spdlog

### Memory Management

- Prefer stack allocation over heap allocation
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) when heap allocation is necessary
- Avoid raw pointers when ownership semantics are unclear

## Testing

- Write unit tests for new functionality
- Ensure existing tests pass before submitting a PR
- Use Google Test framework for testing

## Documentation

- Document public APIs with Doxygen-style comments
- Update README.md if you add new features
- Keep documentation up-to-date with code changes

## Pull Request Guidelines

- Keep PRs focused on a single feature or bug fix
- Write clear commit messages
- Ensure your code passes all tests
- Update documentation as needed
- Be responsive to review comments

## Feature Requests

If you have a feature request, please:

1. Check if it's already been requested in the [issues](https://github.com/your-username/quantclaw/issues)
2. If not, open a new issue with:
   - A clear description of the feature
   - The problem it solves
   - Any relevant examples or use cases

## Bug Reports

When reporting bugs, please include:

1. Your operating system and compiler version
2. Steps to reproduce the issue
3. Expected behavior
4. Actual behavior
5. Any relevant error messages or logs

## License

By contributing to QuantClaw, you agree that your contributions will be licensed under the Apache License 2.0.

---

Thank you for contributing to QuantClaw! 🦞