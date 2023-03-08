EXEC = spl
CC=gcc
SRCDIR = src
OBJDIR = obj
TESTDIR = test
TESTBIN = bin

SRCS = $(wildcard $(SRCDIR)/*.c)
SRCS_NOMAIN = $(wildcard $(SRCDIR)/spl_*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
OBJS_NOMAIN = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS_NOMAIN))
TESTS = $(wildcard $(TESTDIR)/*.c)
TESTEXEC = $(patsubst $(TESTDIR)/%.c,$(TESTDIR)/$(TESTBIN)/%,$(TESTS))

CFLAGS = -g -Wall
INCL = 

build: $(OBJDIR) $(EXEC)
test: $(OBJDIR) $(TESTDIR)/$(TESTBIN) $(OBJS_NOMAIN) $(TESTEXEC)
	for test in $(TESTEXEC) ; do  echo Run test from: $$test && ./$$test ; done

clean: 
	rm -rf $(OBJS) $(TESTEXEC) $(EXEC) $(TESTDIR)/$(TESTBIN)/* $(TESTDIR)/$(TESTBIN) $(OBJDIR)

$(TESTDIR)/$(TESTBIN)/%: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) $(INCL) $< $(OBJS_NOMAIN) -o $@

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(INCL) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

$(TESTDIR)/$(TESTBIN):
	mkdir $(TESTDIR)/$(TESTBIN)