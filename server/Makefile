# Variablen definieren
# CROSS_COMPILE kann von der Kommandozeile übergeben werden, z.B.
# make CROSS_COMPILE=aarch64-none-linux-gnu-
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -g -Werror
TARGET = aesdsocket
SRC = aesdsocket.c
OBJ = $(SRC:.c=.o)
# LDFLAGS für POSIX-Threads und Realtime-Bibliotheken, falls nötig
LDFLAGS ?= -pthread -lrt

# Standard-Target: Dies ist das Ziel, das ausgeführt wird, wenn du einfach nur make ohne Argumente eingibst. 
default: all

all: $(TARGET)

# Linken der Objektdateien zum ausführbaren Programm
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Kompilieren der .c Datei in eine .o Objektdatei
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean-Target: Wird ausgeführt, wenn man make clean ausführt
clean:
	rm -f $(TARGET) *.o

# Kennzeichnung als "Phony"-Targets (keine echten Dateien). Dies sagt make, dass diese Namen keine echten Dateien auf der Festplatte sind,
# sondern nur Befehlsnamen. Das verhindert Probleme, falls es zufällig eine Datei geben sollte, die "clean" heißt.
.PHONY: all clean default