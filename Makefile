TARGET = tui

DEV_CFLAGS = -g -fsanitize=address,undefined -fno-omit-frame-pointer
DEV_LDFLAGS = -fsanitize=address,undefined

TSAN_CFLAGS = -g -O1 -fsanitize=thread -fno-omit-frame-pointer
TSAN_LDFLAGS = -fsanitize=thread

PROD_CFLAGS = -O2
PROD_LDFLAGS =

CFLAGS = -MMD -MP
LDFLAGS =

dev: CFLAGS += $(DEV_CFLAGS)
dev: LDFLAGS += $(DEV_LDFLAGS)
dev: $(TARGET)

tsan: CFLAGS += $(TSAN_CFLAGS)
tsan: LDFLAGS += $(TSAN_LDFLAGS)
tsan: $(TARGET)

prod: CFLAGS += $(PROD_CFLAGS)
prod: LDFLAGS += $(PROD_LDFLAGS)
prod: $(TARGET)

RENDERDIR = ../OIB/

# Linking
$(TARGET): TUI.h libTUI.a OIB.h libOIB.a libMoltnCore.a libHelper.a  main.o  
	gcc main.o -o $@ $(LDFLAGS) libTUI.a libOIB.a libMoltnCore.a libHelper.a -lm

libHelper.a:
	$(MAKE) -C $(RENDERDIR)
	cp ../FormNetwork/libHelper.a .

libMoltnCore.a:
	$(MAKE) -C $(RENDERDIR)
	cp $(RENDERDIR)libMoltnCore.a .

libOIB.a:
	$(MAKE) -C $(RENDERDIR)
	cp $(RENDERDIR)libOIB.a .

OIB.h:
	$(MAKE) -C $(RENDERDIR)
	cp $(RENDERDIR)OIB.h .

TUI.h: OIB.h
	@echo "Generating TUI headers"
	@echo "#pragma once" > TUI.h
	@cat OIB.h input.h keys.h output.h >> TUI.h

# Static lib
libTUI.a: input.o output.o keys.o 
	ar rs $@ $^

# Compiling
main.o: main.c
	gcc $(CFLAGS) -c main.c -o $@

output.o: output.c output.h
	gcc $(CFLAGS) -c output.c -o $@


# Terminal input
input.o: input.c input.h
	gcc $(CFLAGS) -c input.c -o $@

keys.o: keys.c keys.h
	gcc $(CFLAGS) -c keys.c -o $@



# tools
clean:
	rm -f *.o *.a *.d

fclean:
	rm -f $(TARGET) *.o *.a *.d TUI.h OIB.h

fixTerminal:
	stty sane

# merges .d files into dependency graph
-include *.d
