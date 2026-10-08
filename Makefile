BIN    := miku
SRC    := main.c widget.c
CC     ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=gnu11
LDLIBS := -lraylib -lm -lpthread -ldl -lrt -lGL

# Desktop widget hints need Xlib. Everything else (transparent, click-through) is handled
# by raylib itself, so a Wayland-only or DRM build still compiles and runs.
HAVE_X11 := $(shell printf '\043include <X11/Xlib.h>\nint main(void){return 0;}\n' | $(CC) -x c -fsyntax-only - >/dev/null 2>&1 && echo yes)

ifeq ($(HAVE_X11),yes)
	CFLAGS += -DWIDGET_USE_X11
	LDLIBS += -lX11
endif

$(BIN): $(SRC) widget.h
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDLIBS)

clean:
	rm -f $(BIN)

.PHONY: clean