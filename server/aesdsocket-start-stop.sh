#!/bin/sh

# Name der ausführbaren Datei
DAEMON="aesdsocket"
# Pfad zur ausführbaren Datei (auf dem Zielsystem meist /usr/bin oder /bin)
DAEMON_PATH="/usr/bin/$DAEMON"
# Argumente: -d für den Daemon-Modus
DAEMON_ARGS="-d"

case "$1" in
    start)
        echo "Starting $DAEMON..."
        # --start: Startet den Prozess, falls er noch nicht läuft
        # --exec: Pfad zum Programm
        # -- : Trennt Daemon-Optionen von Programm-Argumenten
        start-stop-daemon --start --exec "$DAEMON_PATH" -- $DAEMON_ARGS
        ;;
    stop)
        echo "Stopping $DAEMON..."
        # --stop: Beendet den Prozess
        # --signal TERM: Sendet SIGTERM (Standard für start-stop-daemon)
        # --name: Sucht den Prozess anhand des Namens
        start-stop-daemon --stop --signal TERM --name "$DAEMON"
        ;;
    restart)
        $0 stop
        sleep 1
        $0 start
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac

exit 0