#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#ifndef _WIN32
void blockSignal() {
        struct sigaction sa;
        sa.sa_handler = SIG_IGN;
        sa.sa_flags = 0;
        if (sigemptyset(&sa.sa_mask) == -1 ||
                sigaction(SIGPIPE, &sa, 0) == -1) {
                perror("failed to ignore SIGPIPE; sigaction");
                exit(EXIT_FAILURE);
        }

        sigset_t signal_mask;
        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, SIGPIPE);
        int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
        if (rc != 0) {
                LOG::INFO("block sigpipe error\n");
        }
}
#endif // !_WIN32

