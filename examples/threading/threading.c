#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    // 1. Warten, bevor der Mutex angefordert wird
    usleep(thread_func_args->wait_to_obtain_ms * 1000);

    // 2. Mutex sperren --> jz muss jeder andere Thread warten bis der Mutex entsperrt wird
    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if (rc != 0) {
        ERROR_LOG("Mutex lock failed");
        return thread_param;
    }

    // 3. Warten, während der Mutex gehalten wird
    usleep(thread_func_args->wait_to_release_ms * 1000);

    // 4. Mutex wieder freigeben --> jz können andere Threads wieder auf den Mutex zugreifen
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if (rc != 0) {
        ERROR_LOG("Mutex unlock failed");
        return thread_param;
    }

    // Erfolg vermerken
    thread_func_args->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

     // 1. Speicher auf dem Heap reservieren
    struct thread_data* data = (struct thread_data*) malloc(sizeof(struct thread_data));
    if (data == NULL) return false;

    // 2. Struktur ausfüllen
    data->mutex = mutex;    // Mutex wird zuvor in anderer Funktion (beispielsweise main) initialsiert --> alle Threads greifen auf das selbe mutex object zu!
                            // man erstellt z.B. ein Mutex für die USB-Schnittstelle um zu verhindern, dass mehrere Threads gleichzeitig auf die USB Schnittstelle zugreifen
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->thread_complete_success = false;

    // 3. Thread erstellen
    int rc = pthread_create(thread, NULL, threadfunc, data);
    
    if (rc != 0) {
        free(data); // Falls fehlgeschlagen, Speicher aufräumen
        return false;
    }

    return true;
}

