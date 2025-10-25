CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

EXTRACT_SRC = src/extract-buildinfo.c
EXTRACT_BIN = extract-buildinfo

.PHONY: all install clean test

all: $(EXTRACT_BIN)

$(EXTRACT_BIN): $(EXTRACT_SRC)
	$(CC) $(CFLAGS) $< -o $@

install: all
	install -d $(BINDIR)
	install -m 755 buildinfo $(BINDIR)/buildinfo
	install -m 755 $(EXTRACT_BIN) $(BINDIR)/extract-buildinfo
	@echo ""
	@echo "buildinfo installed successfully!"
	@echo ""
	@echo "Usage:"
	@echo "  buildinfo setup <directory>  # Create new or integrate into existing project"
	@echo "  extract-buildinfo <binary>   # Extract metadata from compiled binary"
	@echo ""

uninstall:
	rm -f $(BINDIR)/buildinfo
	rm -f $(BINDIR)/extract-buildinfo

clean:
	rm -f $(EXTRACT_BIN)

# Run a simple test
test: all
	@echo "Running buildinfo test..."
	@mkdir -p test-tmp
	@./buildinfo setup test-tmp
	@echo ""
	@echo "Building test project..."
	@cd test-tmp && make
	@echo ""
	@echo "Testing version flags..."
	@test-tmp/bin/myapp -V
	@echo ""
	@test-tmp/bin/myapp --version
	@echo ""
	@echo "Extracting metadata from binary..."
	@./extract-buildinfo test-tmp/bin/myapp
	@echo ""
	@rm -rf test-tmp
	@echo "Test completed successfully!"
