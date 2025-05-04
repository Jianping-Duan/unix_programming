CC ?= cc
RMF = rm -f

CFLAGS = -Wall -W -O2 -m64 -Wfloat-equal -Wfloat-conversion -Wdouble-promotion
CFLAGS += $(DEBUG)
CFLAGS += $(CFLAGS_AUX)

INCLUDE_PATH = -I $(TOPDIR)/include
BINARY_PATH = $(TOPDIR)/bin

.PHONY: all
all: $(SLIBS) $(EXECS)

$(EXECS): $(.PREFIX).c
	$(CC) $(.ALLSRC) -o $(BINARY_PATH)/$(.TARGET) $(CFLAGS) $(INCLUDE_PATH)
	
.PHONY: clean
clean:
.for n in $(EXECS)
	$(RMF) $(BINARY_PATH)/$(n)
.endfor
