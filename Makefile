# © 2011 Nicolai Waniek
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the 
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name(s) of the above copyright
# holders shall not be used in advertising or otherwise to promote the sale,
# use or other dealings in this Software without prior written authorization.

-include local.mk

CC         = gcc
CPPC       = g++
CSTD       = c99
CPPSTD     = c++0x
CWARNINGS  = -Wall -Wextra -Wpointer-arith -Wcast-qual -Wswitch-default       \
	     -Wcast-align -Wundef -Wno-empty-body -Wreturn-type -Wformat -W   \
	     -Wtrigraphs -Wno-unused-function -Wmultichar -Wparentheses       \
	     -Wchar-subscripts
CPPWARNINGS= $(CWARNINGS) -Woverloaded-virtual

INCLUDES   = -Iinclude -I/usr/include/OpenEXR -I/usr/include/opencv
LIBS       = -pthread -lm -lIlmImf -lImath -lIex -lIlmThread -lopencv_core    \
	     -lopencv_highgui
LDFLAGS    = -L/usr/lib $(LIBS)

CFLAGS     = -pipe -march=native -mtune=native $(INCLUDES) -std=$(CSTD)
CPPFLAGS   = -pipe -march=native -mtune=native $(INCLUDES) -std=$(CPPSTD)

SRC_DIR    = src
SRC_C      = $(shell find $(SRC_DIR) -name \*.c -type f -print)
SRC_CPP    = $(shell find $(SRC_DIR) -name \*.cpp -type f -print)

OBJ_DIR    = build
OBJ_C      = $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(patsubst %.c,%.c.o,$(SRC_C)))
OBJ_CPP    = $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(patsubst %.cpp,%.cpp.o,$(SRC_CPP)))

DEP_C      = $(patsubst %.o,%.d,$(OBJ_C))
DEP_CPP    = $(patsubst %.o,%.d,$(OBJ_CPP))
DIR_TREE   = $(OBJ_DIR) \
	     $(patsubst $(SRC_DIR)/%,%(OBJ_DIR)/%,$(shell find $(SRC_DIR)/* -type d -print))


ifeq ($(USE_WARNINGS),true)
	CFLAGS += $(CWARNINGS)
	CPPFLAGS += $(CPPWARNINGS)
endif
ifeq ($(BE_TEDIOUS),true)
	CWARNINGS += -Werror
	CPPWARNINGS += -Werror
endif

ifeq ($(USE_DEBUG),true)
	CFLAGS += -ggdb -DDEBUG=1
	CPPFLAGS += -ggdb -DDEBUG=1
else
ifeq ($(USE_OPTIMIZATION),true)
	CFLAGS += -O3 -ftree-vectorize
	CPPFLAGS += -O3 -ftree-vectorize
endif 
endif


define link
	@echo -e '\033[1;32m'[LD] $1 '\033[1;m'
	@$(CPPC) -o $1 $^ $2
endef

define compile
	@echo -e '\033[1;34m'[CC] $< '\033[1;m'
	@$1 -o $@ -c $2 $<
endef

define makedep-c
	@$1 -MM -MG -MP -MT "$@" -MF $(subst .o,.d,$@) $2 $<
endef
define makedep-cpp
	@$1 -MM -MG -MP -MT "$@" -MF $(subst .o,.d,$@) $2 $<
endef


all: makedirs exr2pgm exrcvview exrflow

-include $(DEP_C)
-include $(DEP_CPP)

makedirs:
	@mkdir -p $(DIR_TREE)

exr2pgm: $(OBJ_DIR)/exr2pgm.cpp.o
	$(call link, $@, $(LDFLAGS))

exrcvview: $(OBJ_DIR)/exrcvview.cpp.o
	$(call link, $@, $(LDFLAGS))

exrflow: $(OBJ_DIR)/exrflow.cpp.o $(OBJ_DIR)/math.cpp.o
	$(call link, $@, $(LDFLAGS))

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c
	$(call makedep, $(CC), $(CFLAGS))
	$(call compile, $(CC), $(CFLAGS))

$(OBJ_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	$(call makedep, $(CPPC), $(CPPFLAGS))
	$(call compile, $(CPPC), $(CPPFLAGS))

clean:
	@rm -f $(OBJ_C)
	@rm -f $(DEP_C)
	@rm -f $(OBJ_CPP)
	@rm -f $(DEP_CPP)
	@rm -f exr2pgm
	@rm -f exrcvview
	@rm -f exrflow
	@rm -rf build
