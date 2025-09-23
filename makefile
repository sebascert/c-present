lib_name := cpresent

# arguments for running programs
args ?=

# directories
include_dir         := include
src_dir             := src
test_include_dir    := test/include
test_dir            := test
build_dir           := build

# targets
bin_target   := $(build_dir)/$(lib_name)
lib_target   := $(build_dir)/lib$(lib_name).a
test_target  := $(build_dir)/test

# sources and headers
headers      := $(shell find $(include_dir) -name '*.h')
main_source  := main.c
sources      := $(shell find $(src_dir) -name '*.c')
test_sources := $(shell find $(test_dir) -name '*.c')

# objects
main_obj      := $(main_source:.c=.o)
source_objs   := $(sources:.c=.o)
test_objs     := $(test_sources:.c=.o)
objs          := $(source_objs) $(test_objs)

# env setup
CC          := gcc
CSTD        := c99
AR          := ar
CFLAGS      := -Wall -Wextra -g -std=$(CSTD) -I$(include_dir)
LDFLAGS     := -L$(build_dir) -l$(lib_name)
TEST_CFLAGS := -I$(test_include_dir)

MAKEFLAGS += --no-print-directory

CLANGDB   := compile_commands.json

PREFIX    := /usr/local

# target
all: bin

bin: $(bin_target)
lib: $(lib_target)
test: $(test_target)

install: $(lib_target)
	@rm -rf $(PREFIX)/include/$(lib_name)/
	@rsync -R $(headers) $(PREFIX)/
	@cp $(lib_target) $(PREFIX)/lib/

run-test: $(test_target)
	@echo "./$(test_target) $(args)"
	@./$(test_target) $(args)

.PHONY: all bin lib test install run-test

# utils
format:
	@clang-format -i $(headers) $(sources) $(test_sources)

lint: $(CLANGDB)
	@clang-tidy $(headers) $(sources) $(test_sources) -p .

clangdb: clean-clangdb
	@$(MAKE) $(CLANGDB)

.PHONY: format lint clangdb

# compilation
$(bin_target): $(lib_target) $(main_obj) | $(build_dir)
	$(CC) $(CFLAGS) $(main_obj) -o $@ $(LDFLAGS)
	@echo "built bin to: $@"

$(lib_target): $(source_objs) | $(build_dir)
	$(AR) rcs $@ $^
	@echo "built lib to: $@"

$(test_target): $(source_objs) $(test_objs) | $(build_dir)
	$(CC) $(CFLAGS) -lcriterion $(source_objs) $(test_objs) -o $@
	@echo "built tests to: $@"

$(test_dir)/%.o: $(test_dir)/%.c #| $(test_include_dir)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(build_dir):
	@mkdir -p $@

$(test_include_dir):
	@mkdir -p $@

$(CLANGDB):
	@$(MAKE) clean
	@bear -- $(MAKE) $(objs)

# clean
clean:
	@find . -name '*.o' -delete
	@rm -rf $(build_dir)

clean-clangdb:
	@rm -f $(CLANGDB)
	@rm -rf .cache/clangd

clean-all: clean clean-clangdb

.PHONY: clean clean-clangdb clean-all
