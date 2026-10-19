CC     = gcc
AR     = ar
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -g
PREFIX = /usr/local

# Recompile do zero ao ligar ou desligar os sanitizers.
ifeq ($(SANITIZE),1)
CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address,undefined
endif

# O GCC gera arquivos .d com as dependências dos headers.
CPPFLAGS += -Iinclude -MMD -MP

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

# Mantém os objetos dos testes para não recompilá-los sem necessidade.
.SECONDARY: $(TEST_OBJS)

all: lib $(APP_BINS)

lib: lib/libagenda.a lib/libagenda.so

build/static/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# A biblioteca dinâmica precisa de objetos compilados com -fPIC.
build/shared/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@

lib/libagenda.a: $(STATIC_OBJS)
	@mkdir -p lib
	$(AR) rcs $@ $^

lib/libagenda.so: $(SHARED_OBJS)
	@mkdir -p lib
	$(CC) -shared $(LDFLAGS) -o $@ $^

build/app/%.o: app/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

bin/agenda_static: $(APP_OBJ) lib/libagenda.a
	@mkdir -p bin
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJ) lib/libagenda.a

# O carregador procura a biblioteca em ../lib, a partir do executável.
bin/agenda_shared: $(APP_OBJ) lib/libagenda.so
	@mkdir -p bin
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJ) -Llib -lagenda -Wl,-rpath,'$$ORIGIN/../lib'

run: bin/agenda_static
	./bin/agenda_static --demo

run-shared: bin/agenda_shared
	./bin/agenda_shared --demo

build/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

bin/tests/%: build/tests/%.o lib/libagenda.a
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $< lib/libagenda.a

# Executa todos os programas, mesmo que algum falhe.
test: $(TEST_BINS)
	@failed=0; \
	for t in $(TEST_BINS); do ./$$t || failed=1; done; \
	if [ $$failed -eq 0 ]; then echo "ALL TESTS PASSED"; \
	else echo "SOME TESTS FAILED"; exit 1; fi

sanitize:
	$(MAKE) clean
	$(MAKE) test SANITIZE=1; rc=$$?; \
	$(MAKE) clean; exit $$rc

# PREFIX permite instalar sem privilégios; DESTDIR permite uma instalação temporária.
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

# Lê as dependências; na primeira compilação, os arquivos ainda não existem.
-include $(STATIC_OBJS:.o=.d) $(SHARED_OBJS:.o=.d) $(APP_OBJ:.o=.d) $(TEST_OBJS:.o=.d)
