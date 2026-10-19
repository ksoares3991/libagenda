# libagenda - weekly student schedule library (conflicts and free time)
#
# Main targets:  all  clean  run  test  install        (see `make help`)
#
# Layout:  include/ public headers      src/   library modules
#          app/     client program      tests/ test programs (tests/test_*.c)
#          build/   object files        lib/   libagenda.a, libagenda.so
#          bin/     executables

CC     = gcc
AR     = ar
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -g
PREFIX = /usr/local

# SANITIZE=1 builds with AddressSanitizer + UndefinedBehaviorSanitizer.
# Run `make clean` when switching it on or off: objects are not rebuilt automatically.
ifeq ($(SANITIZE),1)
CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address,undefined
endif

# -MMD -MP make GCC write a .d file next to each object listing the headers it
# included. They are read back at the end of this Makefile, which is how a change
# in a header recompiles exactly the files that include it (incremental build).
CPPFLAGS += -Iinclude -MMD -MP

# ---- file lists (discovered automatically) ---------------------------------
LIB_SRCS    := $(wildcard src/*.c)
STATIC_OBJS := $(LIB_SRCS:src/%.c=build/static/%.o)
SHARED_OBJS := $(LIB_SRCS:src/%.c=build/shared/%.o)

APP_SRC  := $(wildcard app/main.c)
APP_OBJ  := $(APP_SRC:app/%.c=build/app/%.o)
APP_BINS := $(if $(APP_SRC),bin/agenda_static bin/agenda_shared)

TEST_SRCS := $(wildcard tests/test_*.c)
TEST_OBJS := $(TEST_SRCS:tests/%.c=build/tests/%.o)
TEST_BINS := $(TEST_SRCS:tests/%.c=bin/tests/%)

.PHONY: all lib clean run run-shared test sanitize install uninstall help

# Keep the test objects: without this, make deletes them after linking the test
# programs and recompiles every test on each `make test`.
.SECONDARY: $(TEST_OBJS)

all: lib $(APP_BINS)

lib: lib/libagenda.a lib/libagenda.so

# ---- library objects ---------------------------------------------------------
# Static library objects are plain; shared library objects need -fPIC
# (position-independent code) because the .so is mapped at any address.
build/static/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/shared/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@

# ---- libraries ---------------------------------------------------------------
lib/libagenda.a: $(STATIC_OBJS)
	@mkdir -p lib
	$(AR) rcs $@ $^

lib/libagenda.so: $(SHARED_OBJS)
	@mkdir -p lib
	$(CC) -shared $(LDFLAGS) -o $@ $^

# ---- client application ------------------------------------------------------
build/app/%.o: app/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Linked against the static library: the library code is copied into the executable.
bin/agenda_static: $(APP_OBJ) lib/libagenda.a
	@mkdir -p bin
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJ) lib/libagenda.a

# Linked against the shared library: the executable only records a dependency on
# libagenda.so and the dynamic loader resolves it at run time (see `ldd`).
# The rpath makes the loader also look in ../lib relative to the executable, so
# the program runs without setting LD_LIBRARY_PATH, from the build tree or after
# `make install`.
bin/agenda_shared: $(APP_OBJ) lib/libagenda.so
	@mkdir -p bin
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJ) -Llib -lagenda -Wl,-rpath,'$$ORIGIN/../lib'

# ---- run ---------------------------------------------------------------------
run: bin/agenda_static
	./bin/agenda_static --demo

run-shared: bin/agenda_shared
	./bin/agenda_shared --demo

# ---- tests -------------------------------------------------------------------
build/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

bin/tests/%: build/tests/%.o lib/libagenda.a
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $< lib/libagenda.a

# Runs every test program, never stops at the first failure, and exits with a
# non-zero status if any program failed.
test: $(TEST_BINS)
	@failed=0; \
	for t in $(TEST_BINS); do ./$$t || failed=1; done; \
	if [ $$failed -eq 0 ]; then echo "ALL TESTS PASSED"; \
	else echo "SOME TESTS FAILED"; exit 1; fi

# Full test run under the sanitizers, from a clean tree, leaving the tree clean
# (even when a test fails).
sanitize:
	$(MAKE) clean
	$(MAKE) test SANITIZE=1; rc=$$?; \
	$(MAKE) clean; exit $$rc

# ---- install -----------------------------------------------------------------
# Example without administrator rights:  make install PREFIX=$HOME/.local
# Headers go to $(PREFIX)/include/agenda/ (use -I$(PREFIX)/include/agenda).
install: all
	install -d $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include/agenda $(DESTDIR)$(PREFIX)/bin
	install -m 644 lib/libagenda.a $(DESTDIR)$(PREFIX)/lib/
	install -m 755 lib/libagenda.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 include/*.h $(DESTDIR)$(PREFIX)/include/agenda/
	if [ -f bin/agenda_shared ]; then install -m 755 bin/agenda_shared $(DESTDIR)$(PREFIX)/bin/agenda; fi
	@echo "Installed to $(DESTDIR)$(PREFIX)"

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/libagenda.a $(DESTDIR)$(PREFIX)/lib/libagenda.so
	rm -rf $(DESTDIR)$(PREFIX)/include/agenda
	rm -f $(DESTDIR)$(PREFIX)/bin/agenda

# ---- housekeeping ------------------------------------------------------------
clean:
	rm -rf build lib bin

help:
	@echo "make              build libraries and applications (same as: make all)"
	@echo "make run          run the demo with the statically linked program"
	@echo "make run-shared   run the demo with the dynamically linked program"
	@echo "make test         build and run every test; prints ALL TESTS PASSED or fails"
	@echo "make sanitize     run the tests with AddressSanitizer and UBSan"
	@echo "make install      install to PREFIX (default /usr/local), e.g. PREFIX=\$$HOME/.local"
	@echo "make uninstall    remove what install copied"
	@echo "make clean        remove build/, lib/ and bin/"

# Header dependencies generated by -MMD (missing on the first build, hence '-').
-include $(STATIC_OBJS:.o=.d) $(SHARED_OBJS:.o=.d) $(APP_OBJ:.o=.d) $(TEST_OBJS:.o=.d)
