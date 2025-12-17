#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    // 1. Initialization of syslog to be able to log to syslog
    openlog("writer-app", LOG_PID, LOG_USER);

    // 2. Argumentprüfung
    // argc muss 3 sein (Programmname, Pfad, Text)
    if (argc != 3) {
        syslog(LOG_ERR, "Fehler: Ungültige Anzahl an Argumenten. Erwartet: 2, Erhalten: %d", argc - 1);
        fprintf(stderr, "Verwendung: %s <Dateipfad> <Text>\n", argv[0]);
        closelog();
        return 1;
    }

    const char *writefile = argv[1];
    const char *writestr = argv[2];

    // 3. Protokollierung des Schreibvorgangs (LOG_DEBUG)
    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

    // 4. Datei öffnen
    // O_WRONLY: Nur zum Schreiben öffnen
    // O_CREAT: Datei erstellen, falls nicht vorhanden
    // O_TRUNC: Vorhandene Datei leeren
    // 0644: Berechtigungen (rw-r--r--)
    int fd = open(writefile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        syslog(LOG_ERR, "Fehler beim Öffnen/Erstellen der Datei %s: %s", writefile, strerror(errno));
        closelog();
        return 1;
    }

    // 5. In die Datei schreiben
    ssize_t nr = write(fd, writestr, strlen(writestr));
    if (nr == -1) {
        syslog(LOG_ERR, "Fehler beim Schreiben in die Datei: %s", strerror(errno));
        close(fd);
        closelog();
        return 1;
    } else if (nr < strlen(writestr)) {
        syslog(LOG_ERR, "Teilschreibfehler: Nicht alle Bytes wurden geschrieben.");
        close(fd);
        closelog();
        return 1;
    }

    // 6. Ressourcen aufräumen
    close(fd);
    //close syslog
    closelog();

    return 0;
}