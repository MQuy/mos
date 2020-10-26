ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
  else # in a bash-like shell, like msys
	CLEANUP = rm -f
	MKDIR = mkdir -p
  endif
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=out
endif

.PHONY: clean
.PHONY: test

PATHU = unity/src/
PATHS = ../src/
PATHT = ./
PATHB = build/
PATHD = build/depends/
PATHO = build/objs/
PATHR = build/results/

BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHR)

SRCT = $(wildcard $(PATHT)*.c)

COMPILE=gcc -c
LINK=gcc
DEPEND=gcc -MM -MG -MF
CFLAGS=-I. -I$(PATHU) -I$(PATHS) -DTEST

RESULTS = $(patsubst $(PATHT)%.c,$(PATHR)%.txt,$(SRCT) )

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

$(PATHB)%.$(TARGET_EXTENSION): $(PATHO)%.o $(PATHU)unity.o #$(PATHD)Test%.d
	$(LINK) -o $@ $^

$(PATHO)%.o:: $(PATHT)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)%.o:: $(PATHS)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)%.o:: $(PATHU)%.c $(PATHU)%.h
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHD)%.d:: $(PATHT)%.c
	$(DEPEND) $@ $<

$(PATHB):
	$(MKDIR) $(PATHB)

$(PATHD):
	$(MKDIR) $(PATHD)

$(PATHO):
	$(MKDIR) $(PATHO)

$(PATHR):
	$(MKDIR) $(PATHR)

clean:
	$(CLEANUP) $(PATHO)*.o
	$(CLEANUP) $(PATHB)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)*.txt

.PRECIOUS: $(PATHB)Test%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHD)%.d
.PRECIOUS: $(PATHO)%.o
.PRECIOUS: $(PATHR)%.txt
