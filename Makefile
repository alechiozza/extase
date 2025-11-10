TARGET = extase
SRC_DIR = src
BUILD_ROOT = build
ARGS ?= test.c

BUILD_DIR_RELEASE = $(BUILD_ROOT)/release
BUILD_DIR_DEBUG = $(BUILD_ROOT)/debug

CC = gcc
LDFLAGS =

INCLUDE_DIRS = $(shell find $(SRC_DIR) -type d)
IFLAGS = $(addprefix -I,$(INCLUDE_DIRS))

COMMON_CFLAGS = -Wall -Wextra -MMD -MP
RELEASE_CFLAGS = -O2
DEBUG_CFLAGS = -g -O0

VALGRIND = valgrind
VALGRIND_LOG = valgrind-report.log
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=$(VALGRIND_LOG)

SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c)

OBJS_RELEASE = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR_RELEASE)/%.o)
DEPS_RELEASE = $(OBJS_RELEASE:%.o=%.d)
TARGET_RELEASE = $(BUILD_DIR_RELEASE)/$(TARGET)

OBJS_DEBUG = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR_DEBUG)/%.o)
DEPS_DEBUG = $(OBJS_DEBUG:%.o=%.d)
TARGET_DEBUG = $(BUILD_DIR_DEBUG)/$(TARGET)

.PHONY: all
all: $(TARGET_RELEASE)
	@echo "Build completed (Release): $(TARGET_RELEASE)"

.PHONY: debug
debug: $(TARGET_DEBUG)
	@echo "Build completed (Debug): $(TARGET_DEBUG)"

$(TARGET_RELEASE): $(OBJS_RELEASE)
	@echo "LD  $@ (Release)"
	@mkdir -p $(@D)
	$(CC) $(OBJS_RELEASE) -o $@ $(LDFLAGS)

$(BUILD_DIR_RELEASE)/%.o: $(SRC_DIR)/%.c
	@echo "CC  $@ (Release)"
	@mkdir -p $(@D)
	$(CC) $(COMMON_CFLAGS) $(RELEASE_CFLAGS) $(IFLAGS) -c $< -o $@

$(TARGET_DEBUG): $(OBJS_DEBUG)
	@echo "LD  $@ (Debug)"
	@mkdir -p $(@D)
	$(CC) $(OBJS_DEBUG) -o $@ $(LDFLAGS)

$(BUILD_DIR_DEBUG)/%.o: $(SRC_DIR)/%.c
	@echo "CC  $@ (Debug)"
	@mkdir -p $(@D)
	$(CC) $(COMMON_CFLAGS) $(DEBUG_CFLAGS) $(IFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@echo "Cleaning all build artifacts..."
	rm -rf $(BUILD_ROOT)

.PHONY: run
run: all
	@echo "Running release build..."
	./$(TARGET_RELEASE) $(ARGS)

.PHONY: memcheck
memcheck: debug
	@echo "Running Valgrind on debug build..."
	@echo "Log will be written to $(VALGRIND_LOG)"
	@rm -f $(VALGRIND_LOG)
	$(VALGRIND) $(VALGRIND_FLAGS) ./$(TARGET_DEBUG) $(ARGS)
	@echo "--- Valgrind Finished. Quick Report: ---"
	@grep -A 7 "LEAK SUMMARY" $(VALGRIND_LOG)
	@echo "------------------------------------------"
	@echo "Full report is in $(VALGRIND_LOG)"

.PHONY: count
count:
	@python3 count.py

-include $(DEPS_RELEASE)
-include $(DEPS_DEBUG)
