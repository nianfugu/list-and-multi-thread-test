EXE := thread_test-x86

COMMON_LIB := /home/gunianfu/work/test
COMMON_BIN := /home/gunianfu/work/test/bin

#CC := /home/gunianfu/bin/arm-2014.05/bin/arm-none-linux-gnueabi-gcc
#CC := arm-none-linux-gnueabi-gcc
CC := gcc

#SRC=$(wildcard *.c)
SRC := thread.c list.c
#OBJ := $(SRC:.c=.o)
OBJ := $(patsubst %.c,%.o,$(SRC))
#DEP := $(patsubst %.c,.%.d,$(SRC))

CFLAGS := -g -static -O3
#CFLAGS += -I$(COMMON_LIB)/libv4l/arm_static/include
CFLAGS += -DMULTITHREAD_TEST

LIBS := -lrt -lm -lpthread

$(EXE):$(OBJ)
	$(CC) $(CFLAGS) $^ -o $(COMMON_BIN)/$@ $(LIBS)

$(DEP):.%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM  $< > $@.$$$$; \
	sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEP)
clean:
	@rm $(EXE) $(OBJ) $(DEP) -f
