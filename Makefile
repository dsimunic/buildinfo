CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2

VERSION_FILE = VERSION

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

EXTRACT_SRC = src/extract-buildinfo.c
EXTRACT_BIN = extract-buildinfo
BUILDDIR ?= build

.PHONY: all install clean test

all: $(EXTRACT_BIN)

include buildinfo.mk

BUILDINFO_SRC = $(BUILDDIR)/buildinfo.c

$(BUILDINFO_SRC): buildinfo.mk $(VERSION_FILE)
	$(MAKE) -f buildinfo.mk generate-buildinfo \
		BUILDDIR=$(BUILDDIR) \
		VERSION_FILE=$(VERSION_FILE) \
		CC=$(CC)

$(BUILDDIR)/buildinfo.o: $(BUILDINFO_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/extract-buildinfo.o: $(EXTRACT_SRC)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXTRACT_BIN): $(BUILDDIR)/extract-buildinfo.o $(BUILDDIR)/buildinfo.o
	$(CC) $(BUILDDIR)/extract-buildinfo.o $(BUILDDIR)/buildinfo.o -o $@

install: all
	install -d "$(HOME)/.local/bin"
	install -m 755 bin/buildinfo "$(HOME)/.local/bin"
	install -m 755 $(EXTRACT_BIN) "$(HOME)/.local/bin/extract-buildinfo"
	install -d "$(HOME)/.local/share/buildinfo"
	install -m 644 templates/* "$(HOME)/.local/share/buildinfo"

install-global: all
	install -d $(BINDIR)
	install -m 755 bin/buildinfo $(BINDIR)/buildinfo
	install -m 755 $(EXTRACT_BIN) $(BINDIR)/extract-buildinfo
	install -d $(PREFIX)/share/buildinfo
	install -m 644 templates/* $(PREFIX)/share/buildinfo
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
	rm -rf $(BUILDDIR)

# Print current version
.PHONY: version
version:
	@$(MAKE) -f buildinfo.mk print-version

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
	@test-tmp/bin/test-tmp -V
	@echo ""
	@test-tmp/bin/test-tmp --version
	@echo ""
	@echo "Extracting metadata from binary..."
	@./extract-buildinfo test-tmp/bin/test-tmp
	@echo ""
	@rm -rf test-tmp
	@echo "Test completed successfully!"
