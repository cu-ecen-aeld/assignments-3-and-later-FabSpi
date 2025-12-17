# Variablen definieren
# CROSS_COMPILE kann von der Kommandozeile übergeben werden
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g
TARGET = writer
SRC = writer.c
OBJ = $(SRC:.c=.o)

# Standard-Target: Erstellt die "writer"-Anwendung
all: $(TARGET)

# Linken der Objektdateien zum ausführbaren Programm
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Kompilieren der .c Datei in eine .o Objektdatei
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean-Target: Entfernt die Anwendung und alle .o-Dateien
clean:
	rm -f $(TARGET) *.o

# Kennzeichnung als "Phony"-Targets (keine echten Dateien)
.PHONY: all clean