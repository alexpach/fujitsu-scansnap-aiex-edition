/*
 * scan-button-watch: wait for the ScanSnap's Scan button and run a command.
 *
 * The iX500 has no USB interrupt endpoint, so the button can only be read by
 * asking the scanner. Instead of spawning `scanimage -A` for every check, this
 * keeps one SANE handle open and reads only the "scan" sensor option, which is
 * a single small USB transaction.
 *
 * While the command runs, the SANE handle is closed so the command can use the
 * scanner, then reopened.
 *
 * Usage: scan-button-watch [-w] [-d device] [-i interval_ms] -- command [args...]
 * Exits non-zero if the scanner goes away (Linux: systemd stops/restarts us).
 * With -w it never exits for that; it waits for the scanner to come back (macOS: launchd
 * keeps one watcher running whether the scanner is on or not).
 * -p only checks that the scanner can be opened (used by -w, see wait_for_scanner).
 *
 * Build: cc -O2 -Wall -o scan-button-watch scan-button-watch.c -lsane
 */
#include <sane/sane.h>
#include <sane/saneopts.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t stop;
static void on_signal(int sig) { (void)sig; stop = 1; }

static void sleep_ms(long ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    while (!stop && nanosleep(&ts, &ts) == -1) {}
}

/* Open the device and find the "scan" sensor option. */
static SANE_Status open_scanner(const char *dev, SANE_Handle *h, SANE_Int *opt) {
    SANE_Status st = sane_open(dev, h);
    if (st != SANE_STATUS_GOOD) return st;
    for (SANE_Int i = 1;; i++) {
        const SANE_Option_Descriptor *d = sane_get_option_descriptor(*h, i);
        if (!d) break;
        if (d->name && strcmp(d->name, "scan") == 0 && d->type == SANE_TYPE_BOOL) {
            *opt = i;
            return SANE_STATUS_GOOD;
        }
    }
    sane_close(*h);
    return SANE_STATUS_UNSUPPORTED;
}

static int run(char **argv) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return -1; }
    if (pid == 0) {
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0) {}
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}


/* -p: can the scanner be opened and its button read right now? Exit status 0 if so. */
static int probe(const char *dev) {
    SANE_Int version, opt;
    SANE_Handle h;
    SANE_Bool pressed;
    if (sane_init(&version, NULL) != SANE_STATUS_GOOD) return 1;
    SANE_Status st = open_scanner(dev, &h, &opt);
    if (st == SANE_STATUS_GOOD) {
        st = sane_control_option(h, opt, SANE_ACTION_GET_VALUE, &pressed, NULL);
        sane_close(h);
    }
    sane_exit();
    return st == SANE_STATUS_GOOD ? 0 : 1;
}

/*
 * -w: wait until the scanner works again, then start over. Call with SANE shut down. Returns
 * only when told to stop.
 *
 * This process never touches SANE again. After the scanner has gone away once or twice, SANE
 * here can no longer open it even after sane_exit() + sane_init(), while a new process can
 * (seen on macOS, cause not found). So each attempt is a new process (ourselves with -p), and
 * when one succeeds we replace ourselves with a fresh copy. Nothing is logged per attempt.
 */
#define WAIT_MS 5000
static void wait_for_scanner(const char *dev, char **self_argv) {
    char *probe_argv[] = { self_argv[0], "-p", "-d", (char *)dev, NULL };
    printf("waiting for the scanner\n");
    for (;;) {
        sleep_ms(WAIT_MS);
        if (stop) return;
        if (run(probe_argv) == 0) {
            printf("scanner found\n");
            execvp(self_argv[0], self_argv);
            perror(self_argv[0]);
            exit(70);       /* can't restart ourselves: let launchd do it */
        }
    }
}

int main(int argc, char **argv) {
    const char *dev = "fujitsu";
    long interval = 1000;
    int wait_mode = 0, probe_mode = 0;
    int c;
    while ((c = getopt(argc, argv, "wpd:i:")) != -1) {
        switch (c) {
        case 'w': wait_mode = 1; break;
        case 'p': probe_mode = 1; break;
        case 'd': dev = optarg; break;
        case 'i': interval = strtol(optarg, NULL, 10); break;
        default: goto usage;
        }
    }
    if (probe_mode) return probe(dev);
    if (optind >= argc || interval < 50) goto usage;
    char **cmd = argv + optind;

    setvbuf(stdout, NULL, _IOLBF, 0);
    signal(SIGTERM, on_signal);
    signal(SIGINT, on_signal);

    SANE_Int version;
    if (sane_init(&version, NULL) != SANE_STATUS_GOOD) {
        fprintf(stderr, "sane_init failed\n");
        return 1;
    }

    SANE_Handle h;
    SANE_Int opt;
    SANE_Status st = open_scanner(dev, &h, &opt);
    if (st != SANE_STATUS_GOOD) {
        fprintf(stderr, "cannot open '%s': %s\n", dev, sane_strstatus(st));
        sane_exit();
        if (!wait_mode) return 2;
        wait_for_scanner(dev, argv);
        return 0;
    }
    printf("watching scan button on '%s' every %ld ms\n", dev, interval);

    int rc = 0, failures = 0;
    while (!stop) {
        SANE_Bool pressed = SANE_FALSE;
        st = sane_control_option(h, opt, SANE_ACTION_GET_VALUE, &pressed, NULL);
        if (st != SANE_STATUS_GOOD) {
            /* Transient errors happen (e.g. waking from power save); give up after a few. */
            fprintf(stderr, "reading button failed: %s\n", sane_strstatus(st));
            if (++failures >= 5) {
                if (!wait_mode) { rc = 3; break; }
                sane_close(h);
                sane_exit();
                wait_for_scanner(dev, argv);
                return 0;
            }
            sleep_ms(2000);
            continue;
        }
        failures = 0;

        if (pressed) {
            printf("button pressed, running %s\n", cmd[0]);
            sane_close(h);
            int r = run(cmd);
            printf("%s exited with %d\n", cmd[0], r);
            sleep_ms(2000); /* debounce */
            if (stop) break;
            st = open_scanner(dev, &h, &opt);
            if (st != SANE_STATUS_GOOD) {
                fprintf(stderr, "cannot reopen '%s': %s\n", dev, sane_strstatus(st));
                sane_exit();
                if (!wait_mode) return 2;
                wait_for_scanner(dev, argv);
                return 0;
            }
            continue;
        }
        sleep_ms(interval);
    }

    sane_close(h);
    sane_exit();
    return rc;

usage:
    fprintf(stderr, "usage: %s [-w] [-p] [-d device] [-i interval_ms] -- command [args...]\n", argv[0]);
    return 64;
}
