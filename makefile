NAME            = pi-email


###################################################################################
# PROJECT FILES
###################################################################################

FILES=         main             \
               mail-file

###################################################################################
# PROJECT SETTINGS
###################################################################################
PROJ_DIR	= .
INCLUDE_PATH	= ./inc

OBJECTS		= $(foreach file,$(FILES),$(addsuffix .o,$(file)))
OBJ_TMP 	= $(foreach file,$(OBJECTS),obj/$(file))
OBJ_LIST	= $(subst $(SPACE),$(COMMA),$(OBJ_TMP))

LL_OPT	:= `libesmtp-config --libs`
CFLAGS := $(CFLAGS) -std=c99 -pedantic -O2 -g -W -Wall `libesmtp-config --cflags`


VPATH = src obj

.PHONY : clean all executable

all: executable


###################################################################################
#  C Compiler - Code generation
###################################################################################

%.o: %.c
		$(CC) $(CFLAGS) -c -I $(INCLUDE_PATH) -o obj/$@  $<

executable:	$(OBJECTS)
		$(CC) -g $(OBJ_LIST) $(LL_OPT) -o $(NAME)


test:		
		echo "CC: $(CC)"
		echo "CFLAGS: $(CFLAGS)"
###################################################################################
#  clean
###################################################################################
clean:
	rm -f obj/*.o
	rm -f $(NAME)
	rm -f core
