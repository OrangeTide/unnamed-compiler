CFLAGS += -Wall -W -g
CPPFLAGS += -DNDEBUG=1
all ::
.PHONY : all clean
#
OBJS_ast := ast.o tok.o parse.o
ast :: $(OBJS_ast)
clean :: ; $(RM) ast $(OBJS_ast)
all :: ast
