# TinyPkg - Updated Build Configuration

## Requirements
- C compiler (gcc/clang)
- git
- wget or curl
- tar
- libyaml-dev (for proper YAML parsing)

## Compilation

```bash
# Default build
make

# Clean build artifacts
make clean

# Clean everything including downloaded packages
make distclean

# Run tests (when available)
make test
```

## Installation

```bash
make install
# Binary installed to ~/.local/bin/tinypkg
```

## Security Features

The updated version includes:

1. **Safe Execution** - No shell injection via `system()`
   - Uses `fork()` and `execvp()` for all external commands
   - Proper argument separation

2. **Input Validation** - Package name validation
   - Alphanumeric + dash + underscore only
   - Path traversal protection
   - Length limits

3. **Database Locking** - File locks on installation database
   - Prevents corruption from concurrent access
   - Atomic operations

4. **Error Handling** - Comprehensive error reporting
   - Function names in error messages
   - Actual strerror() output for system errors
   - Proper cleanup on failure

5. **Path Safety** - Centralized path management
   - No path traversal attacks
   - Consistent handling across modules

## Files Updated

- `common.h` / `common.c` - New shared utilities module
- `repo.h` / `repo.c` - Safe execution, better error handling
- `build.h` / `build.c` - Input validation, file locking, safe execution
- `util.h` / `util.c` - Centralized path management, input validation
- `main.c` - Better usage messages, entry-point validation
- `Makefile` - Build configuration

## Compilation Flags

```c
// Recommended compiler flags
-Wall -Wextra -Werror
-D_POSIX_C_SOURCE=200809L
-std=c99
```

## Known Limitations & Future Work

1. **YAML Parsing** - Currently uses simple text parsing
   - TODO: Integrate libyaml library for robust parsing
   - Will require dependency on libyaml

2. **Single Repository** - Only supports official repo
   - TODO: Complete repo_add/remove implementation
   - TODO: Support multiple repository sources

3. **No Dependency Resolution**
   - TODO: Parse version constraints
   - TODO: Implement dependency graph solving

4. **No Package Signatures**
   - TODO: GPG signature verification
   - TODO: Checksum validation

5. **Limited Build System Support**
   - Currently assumes packages use $PREFIX environment variable
   - TODO: Support autoconf, CMake, meson, etc.

## Testing the Updated Code

```bash
# Build
make clean && make

# Sync repository
./tinypkg repo sync

# List packages
./tinypkg list

# Search for packages
./tinypkg search zlib

# Build a package (requires manifest in repo)
./tinypkg build example

# Install package
./tinypkg install example

# Remove package
./tinypkg remove example
```

## Performance Notes

- First `repo sync` downloads the git repository (~varies by size)
- Subsequent syncs only pull changes
- Builds are cached in ~/.cache/tinypkg/build/
- Clean builds: `rm -rf ~/.cache/tinypkg/build/`
