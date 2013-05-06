CFLAGS += -Wall -W -g
CPPFLAGS += -DNDEBUG=1
all ::
.PHONY : all clean
#
OBJS_lang := lang.o ast.o tok.o parse.o vm.o gen.o
lang :: $(OBJS_lang)
clean :: ; $(RM) lang $(OBJS_lang)
all :: lang
