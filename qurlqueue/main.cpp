#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <getopt.h>

#include "qurlqueue_rpc.h"

/*
static void signal_handler(int sig)
{
    switch(sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGINT:
        case SIGQUIT:
            break;
        default:
            break;
    }
}
*/

void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: qurlqueued options \n");
    fprintf(stream,
            "  -h --help             Display this usage information.\n"
            "  -d --deamon           Run as a daemon\n"
            "  -p --port <num>       TCP port number to listen on(default 7676)\n"
            "  -m --multiple <num>   Threads num(default 64, must greater than 10)\n");

    exit(exit_code);
}


int main(int argc, char *argv[])
{
    int daemon = 0;
    int port = -1;
    int url_queue_default_port = 19854;
    int multiple = 64;
    int help = 0;
    pid_t   pid, sid;

    const char* const short_options = "hdp:m:u";
    const struct option long_options[] = {
        { "help",     0, NULL, 'h' },
        { "daemon",   0, NULL, 'd' },
        { "port",     1, NULL, 'p' },
        { "multiple", 1, NULL, 'm' },
        { NULL,       0, NULL, 0   }
    };

    int next_option;
    do {
        next_option = getopt_long (argc, argv, short_options,
                               long_options, NULL);
        switch (next_option) {
            case 'd':
                daemon = 1;
                break;
            case 'h':
                help = 1;
                print_usage(stdout, 0);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'm':
                multiple = atoi(optarg);
                break;
            case -1:
                break;
            case '?':
                print_usage(stderr, 1);
            default:
                print_usage(stderr, 1);
        }
    } while (next_option != -1);
/*
    if (multiple < 10 ) {
        multiple = 10;
    }
*/
    if (port  <= 0) {
        port = url_queue_default_port;
    }

    if (daemon) {
        pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        umask(0);
        sid = setsid();
        if (sid < 0) {
            exit(EXIT_FAILURE);
        }
    }

    msgpack::rpc::loop lo;
    qurlqueue::QUrlQueueServer svr(lo);
    //lo->add_timer(0.1, 0.001, mp::bind(&qurlqueue::QUrlQueueServer::set_current_time));

    svr.instance.listen("0.0.0.0", port);
    svr.start(multiple);

    return 0;
}
