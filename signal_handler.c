#define _POSIX_SOURCE /* enable struct sigaction, sigemptyset and sigaction */

#include "signal_handler.h"
#include <string.h>
#include <stdlib.h>
#include <signal.h>

static void s_handler(int s)
{
    if (s == SIGTERM || s == SIGINT)
        exit(0);
}

void set_signal_handlers()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = s_handler;
    sigemptyset(&act.sa_mask);

    sigaction(SIGTERM, &act, NULL); /* kill */
    sigaction(SIGINT, &act, NULL);  /* ^C */
}
