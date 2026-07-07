TARGET = tui

LIBDIR = lib/
INCDIR = include/

HELPERDIR = ../HelperFuncs/
HELPERINC = $(HELPERDIR)include/
HELPERLIB = $(HELPERDIR)lib/

MOLTNDIR = ../MOLTN/
MOLTNINC = $(MOLTNDIR)include/
MOLTNLIB = $(MOLTNDIR)lib/

OIBDIR = ../OIB/
OIBINC = $(OIBDIR)include/
OIBLIB = $(OIBDIR)lib/

DEV_CFLAGS = -g -fsanitize=address,undefined -fno-omit-frame-pointer
DEV_LDFLAGS = -fsanitize=address,undefined

TSAN_CFLAGS = -g -O1 -fsanitize=thread -fno-omit-frame-pointer
TSAN_LDFLAGS = -fsanitize=thread

PROD_CFLAGS = -O2
PROD_LDFLAGS =

CFLAGS = -MMD -MP -I$(HELPERINC) -I$(MOLTNINC) -I$(OIBINC) -I$(INCDIR)
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

# Linking
$(TARGET): $(INCDIR)TUI.h $(LIBDIR)libTUI.a $(OIBINC)OIB.h $(OIBLIB)libOIB.a $(MOLTNLIB)libMoltnCore.a $(MOLTNINC)MoltnCore.h $(HELPERLIB)libHelper.a  $(HELPERINC)helper.h main.o  
	gcc main.o -o $@ $(LDFLAGS) $(LIBDIR)libTUI.a -L$(OIBLIB) -lOIB -L$(MOLTNLIB) -lMoltnCore -L$(HELPERLIB) -lHelper -lm

$(HELPERLIB)libHelper.a:
	$(MAKE) -C $(HELPERDIR)

$(MOLTNLIB)libMoltnCore.a:
	$(MAKE) -C $(MOLTNDIR)

$(OIBLIB)libOIB.a:
	$(MAKE) -C $(OIBDIR)

# Static lib
$(LIBDIR)libTUI.a: input.o output.o keys.o | $(LIBDIR)
	ar rs $@ $^

# Compiling
main.o: main.c
	gcc $(CFLAGS) -c main.c -o $@

output.o: output.c $(INCDIR)output.h
	gcc $(CFLAGS) -c output.c -o $@


# Terminal input
input.o: input.c $(INCDIR)input.h
	gcc $(CFLAGS) -c input.c -o $@

keys.o: keys.c $(INCDIR)keys.h
	gcc $(CFLAGS) -c keys.c -o $@


$(LIBDIR):
	mkdir -p $(LIBDIR)

# tools
clean:
	rm -f *.o *.d

fclean:
	rm -f $(TARGET) *.o *.d $(LIBDIR)libTUI.a

fixTerminal:
	stty sane

# merges .d files into dependency graph
-include *.d
