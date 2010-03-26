SRC = crio.c crio_eval.c crio_mem.c
OBJ = $(SRC:%.c=%.o)
LIBCRIO = libcrio.a

TEST_SRC = $(wildcard *Test.c)
TEST_OBJ = $(TEST_SRC:%.c=%.o)
TEST_RUNNER = runtests
TEST_RUNNER_SRC = $(TEST_RUNNER).c

ifndef WITH_R
CC = gcc
CFLAGS = -Wall -g -fPIC
endif

test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

$(TEST_RUNNER): $(TEST_OBJ) $(OBJ) $(TEST_RUNNER_SRC)
	$(CC) $(CFLAGS) -c $(TEST_RUNNER_SRC)
	$(CC) -o $@ $^

$(TEST_RUNNER_SRC): $(TEST_SRC)
	./make-tests.sh > $@

lib: $(LIBCRIO)

$(LIBCRIO): $(OBJ)
	ar -r -c $@ $^
	ranlib $@

clean:
	-@rm -fv $(TEST_OBJ) $(OBJ) $(TEST_RUNNER) \
                 $(TEST_RUNNER_SRC) $(TEST_RUNNER).o \
                 $(LIBCRIO)
