TARGET = extase
SRC_DIR = src
BUILD_DIR = build

CC = gcc
CFLAGS = -Wall -Wextra # -std=c11 -g
CFLAGS += -MMD -MP # Add flags to generate dependency files (.d files)

INCLUDE_DIRS = $(shell find $(SRC_DIR) -type d)
IFLAGS = $(addprefix -I,$(INCLUDE_DIRS))
LDFLAGS =

SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

.PHONY: all
all: $(BUILD_DIR)/$(TARGET)
	@echo "Bomboclat."

$(BUILD_DIR)/$(TARGET): $(OBJS)
	@echo "LD  $@"
	@mkdir -p $(@D)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "CC  $@"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)

.PHONY: run
run: all
	./$(BUILD_DIR)/$(TARGET) test.c

.PHONY: count
count:
	@python3 count.py


-include $(DEPS) # read the .d files to find header dependencies.
