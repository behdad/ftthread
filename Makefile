all: ftthread

MODULES = freetype2
CFLAGS = `pkg-config --cflags $(MODULES)` -O0
LDFLAGS = `pkg-config --libs $(MODULES)` -lpthread

%: %.c
	$(CC) $< -o $@ $(CFLAGS) $(LDFLAGS)
