

# La forma normal de usar este Makefile debería ser correr
# "make test" EN EL DIRECTORIO DE ARRIBA, no en este.
CPPFLAGS+= -I..
LDFLAGS+= `pkg-config --libs check`

TARGET=runner
SOURCES=$(shell echo *.c)

# Already compiled modules
COMMON_OBJECTS=../hierarchy_tree.o
MOCK_OBJECTS=mock_fat_file.o fat_fs_tree.o
vpath fat_fs_tree.c ..
fat_fs_tree.o: CPPFLAGS += -DREPLACE_MOCK=1

# - Every test suit tries to link as few files as possible
test_h_tree_runner: test_hierarchy_tree.o $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

test_fat_tree_runner: test_fat_fs_tree.o $(COMMON_OBJECTS) $(MOCK_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Ejecutar runners
test_ht: test_h_tree_runner
	./$^

test_ft: test_fat_tree_runner
	./$^

.PHONY: all clean test

all: test

clean:
	rm -f $(TARGET) *.o test*.log .depend *~ *_runner

.depend: $(SOURCES)
	$(CC) $(CPPFLAGS) -MM $^ > $@

-include .depend
