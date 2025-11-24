# Makefile for code formatting tasks
# ===================================

.PHONY: format check_format help

CLANG_FORMAT=clang-format

# Default target
help:
	@echo "Available targets:"
	@echo "  format        - Format all C/C++ source files using clang-format"
	@echo "  check_format  - Check if files need formatting (non-destructive)"

# Find all C/C++ source files (excluding build and .git directories)
SOURCES := $(shell find . -type f \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.hh" \) \
	-not -path "./build/*" \
	-not -path "./.git/*")

# Format all source files in-place
format:
	@$(CLANG_FORMAT) -i -style=file $(SOURCES)

# Check if any files need formatting (returns non-zero if formatting is needed)
check_format:
	@$(CLANG_FORMAT) -style=file --dry-run --Werror $(SOURCES)
