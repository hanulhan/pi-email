
OBJECTS = mail-file.o main.o
LIBESMTP = `libesmtp-config --libs`
CFLAGS := $(CFLAGS) -std=c99 -pedantic -O2 -g -W -Wall `libesmtp-config --cflags`

all: mail-file

#mail-file-a: $(OBJECTS)
#	$(CC) -g -static $(OBJECTS) $(LIBESMTP) -o mail-file-a

mail-file: $(OBJECTS)
	$(CC) -g $(OBJECTS) $(LIBESMTP) -o mail-file

clean:
	rm -f *.o core mail-file-a mail-file-so mail-file
