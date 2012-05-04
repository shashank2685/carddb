# if flat file is neede comment it
#COMPILE_FOR=SQL_LITE

ifeq ($(COMPILE_FOR), SQL_LITE)
EXTRA_DEFINES = -DSQLITE_ENABLE_UPDATE_DELETE_LIMIT -DALLOW_DUPLICATES -DSQL_LITE
EXTRA_LIBS = -lpthread -ldl
EXTRA_SRC = sqlite3.c
endif

LIBS=
DEFINES = -DDBG -g
SRC_FILES = dbrec.c main.c
OBJS = $(SRC_FILES:.c=.o) $(EXTRA_SRC:.c=.o)

INCLUDES=-I .
%.o : %.c
	$(CC) -Wno-format -Wno-format-security -c $< $(INCLUDES) $(DEFINES) $(EXTRA_DEFINES)

all : carddb printtree


carddb: $(OBJS)
	$(CC) $(DEFINES) $(INCLUDES) $(EXTRA_DEFINES) -o $@ $^ $(LIBS) $(EXTRA_LIBS) 

printtree: printtree.c
	$(CC) -Wno-format -o $@ $^

clean:
	rm -f *.o carddb printtree
