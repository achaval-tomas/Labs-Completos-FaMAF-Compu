CC := gcc

# To eliminate debugging messages use -DNDEBUG
CFLAGS := -O0 -std=gnu11 -Wall -Werror -Wno-unused-parameter -Werror=vla -g \
	  -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -D_GNU_SOURCE
CPPFLAGS := `pkg-config --cflags glib-2.0`
LDFLAGS :=`pkg-config --libs glib-2.0` -lfuse

export CC
export CFLAGS
export CPPFLAGS
export LDFLAGS

IMAGE_FILEPATH := test.img
MOUNT_POINT := ./mnt
HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
TARGET := fat-fuse

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


test-ht: hierarchy_tree.o
	make -C tests test_ht

test-ft:
	make -C tests test_ft

image:
	cp ../resources/complete.img ./$(IMAGE_FILEPATH)

unmount:
	sudo bash unmount.sh $(MOUNT_POINT)
	sleep 1
	
clean-mount-point:
	rm -rf $(MOUNT_POINT)/*

testfs:
	python3 tests/test.py

mount: $(TARGET) 
	make unmount
	./fat-fuse $(IMAGE_FILEPATH) $(MOUNT_POINT)
	sleep 1

mount-native:
	sudo bash mount.sh native $(MOUNT_POINT) $(IMAGE_FILEPATH)
	sleep 1

clean:
	rm -f $(TARGET) $(OBJECTS) tags cscope*
	make -C tests clean

.PHONY: clean
