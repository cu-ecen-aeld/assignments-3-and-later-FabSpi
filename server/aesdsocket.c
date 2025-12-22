#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

#define PORT "9000"
#define DATA_FILE "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 1024

int server_fd = -1;
int client_fd = -1;
volatile sig_atomic_t keep_running = 1;

// Signal Handler für Graceful Exit
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        keep_running = 0;
        // Beendet das blockierende accept() im Hauptprogramm
    }
}

void cleanup() {
    if (server_fd != -1) close(server_fd);
    if (client_fd != -1) close(client_fd);
    unlink(DATA_FILE); // Löscht die Datei beim Beenden
    closelog();
}

int main(int argc, char **argv) {
    openlog("aesdsocket", LOG_PID, LOG_USER);

    //check if daemon mode was selected by user
    bool daemon_mode = false;
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = true;
    }

    // register signals (Kernel now knows that aesd application wants to execute signal_handler when SIGTINT or SIGTERM occures)
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    sigemptyset(&new_action.sa_mask);

    if (sigaction(SIGTERM, &new_action, NULL) != 0) {
        fprintf(stderr, "Error registering SIGTERM: %s\n", strerror(errno));
        return -1;
    }
    if (sigaction(SIGINT, &new_action, NULL) != 0) {
        fprintf(stderr, "Error registering SIGINT: %s\n", strerror(errno));
        return -1;
    }

    //Socket Setup (getaddrinfo)
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //returns a list of all possible addrinfo structs possible on the target hardware
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        syslog(LOG_ERR, "getaddrinfo failed");
        return -1;
    }

    //initialize socket
    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_fd == -1) {
        freeaddrinfo(res);
        return -1;
    }

    // Port wiederverwendbar machen (hilft bei schnellen Neustarts)
    //Wenn du dein Programm zum Testen mit Ctrl+C beendest und sofort wieder 
    // ./aesdsocket tippst, wäre der Port ohne SO_REUSEADDR noch für 1–2 Minuten vom Kernel gesperrt.
    // Durch diese Zeile sagst du dem Betriebssystem: "Es ist okay, wenn ich diesen Port sofort wieder belege, 
    // auch wenn die alte Verbindung technisch noch im TIME_WAIT-Status ist".
    int yes = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    //bind socket to specific IP and port
    if (bind(server_fd, res->ai_addr, res->ai_addrlen) != 0) {
        syslog(LOG_ERR, "Bind failed: %s", strerror(errno));
        freeaddrinfo(res);
        close(server_fd);
        return -1;
    }

    freeaddrinfo(res); // Liste wird nicht mehr benötigt

    // Daemon Mode Fork (ERST NACH BIND)
    if (daemon_mode) {
        pid_t pid = fork(); // creates new child process
        if (pid < 0) exit(-1);
        if (pid > 0) exit(0); // Elternprozess beenden

        setsid();
        chdir("/");
        // Durch die Umleitung auf /dev/null stellst du sicher, dass dein Daemon isoliert ist. Er „belästigt“ niemanden mit Textausgaben im Terminal 
        // und stürzt nicht ab, wenn er keine Eingaben bekommt. Deine einzige Kommunikationsleitung nach außen bleibt syslog, weshalb du dort alle 
        // wichtigen Infos (Verbindungen, Signale) protokollieren musst.
        int devnull = open("/dev/null", O_RDWR);
        dup2(devnull, STDIN_FILENO); //STDIN_FILENO: Schließt die Tastatureingabe.
        dup2(devnull, STDOUT_FILENO); //STDOUT_FILENO & STDERR_FILENO: Schließt die normale Textausgabe und Fehlerausgabe im Terminal.
        dup2(devnull, STDERR_FILENO);
        close(devnull);
    }

    if (listen(server_fd, 10) != 0) return -1;

    // 4. Hauptschleife
    while (keep_running) {
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

        if (client_fd == -1) {
            if (errno == EINTR) break; // Signal hat accept unterbrochen
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        // Daten empfangen und in Datei speichern
        FILE *fp = fopen(DATA_FILE, "a+");
        if (!fp) {
            close(client_fd);
            continue;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_recv;
        while ((bytes_recv = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, 1, bytes_recv, fp);
            if (memchr(buffer, '\n', bytes_recv)) break; // Paketende
        }

        // Gesamten Dateiinhalt zurücksenden
        fseek(fp, 0, SEEK_SET);
        while (fgets(buffer, BUFFER_SIZE, fp)) {
            send(client_fd, buffer, strlen(buffer), 0);
        }

        fclose(fp);
        close(client_fd);
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }

    cleanup();
    return 0;
}