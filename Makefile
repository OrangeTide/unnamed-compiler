CFLAGS += -Wall -W -g
all :: ast
clean :: ; $(RM) ast
.PHONY : all clean
