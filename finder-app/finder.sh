#!/bin/sh

# Die beiden Laufzeitargumente zuweisen, um den Code lesbarer zu machen
filesdir="$1"
searchstr="$2"

# 1. Argumentprüfung: Beendet sich mit dem Fehlerwert 1, wenn nicht beide Argumente angegeben wurden.
if [ -z "$filesdir" ] || [ -z "$searchstr" ]; then
    echo "Fehler: Es werden zwei Argumente benötigt."
    echo "Verwendung: $0 <Verzeichnis-Pfad> <Such-Text>"
    exit 1
fi

# 2. Verzeichnisprüfung: Beendet sich mit dem Fehlerwert 1, wenn filesdir kein Verzeichnis ist.
# -d testet, ob die Variable ein Verzeichnis ist.
if [ ! -d "$filesdir" ]; then
    echo "Fehler: Das angegebene Verzeichnis '$filesdir' existiert nicht oder ist kein Verzeichnis."
    echo "Bitte geben Sie einen gültigen Pfad zu einem Verzeichnis an."
    exit 1
fi

# 3. Hauptlogik: Zählen der Dateien und der übereinstimmenden Zeilen.

# find "$filesdir" -type f: findet im angebenen Ordner alle Objekte, die nur Dateien sind (-type f)
# | concatenation von Befehlen --> Liste von find wird an wc weitergeleitet
#  wc -l: zählt die Anzahl der Zeilen in der erhaltenen Liste
X=$(find "$filesdir" -type f | wc -l)

# grep -r "$searchstr" "$filesdir": durchsucht rekursiv (-r) (also auch in auch in Unterordnern) das gesamte
# $filesdir nach dem Text $searchstr und leitet die erhaltene Liste weiter
# zählt die Anzahl der Zeilen (-l) in der Liste
Y=$(grep -r "$searchstr" "$filesdir" | wc -l)


# 4. Ausgabe der Ergebnisse
# gibt die formatierte Meldung aus.
echo "The number of files are $X and the number of matching lines are $Y"