#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <getopt.h>

#include <map>

#include "qws_config.h"
#include "qws.h"

static void print_usage(const char *program, int exit_code) {

    FILE* stream;
    if (exit_code != 0) {
        stream = stderr;
    } else {
        stream = stdout;
    }

    fprintf(stream, "Usage: %s options \n", program);
    fprintf(stream,
            "  -h  --help                                   Display this usage information.\n"
            "  -t  --tokenizer_type=<viterbi|mfm|unigram>   Tokenizer type.\n"
            "  -m  --mfm_word_dict=<file>                   Word dict file used by MFM Tokeinzer type.\n"
            "  -v  --viterbi_word_dict=<file>               Word dict file used by Viterbi Tokeinzer type.\n"
            "  -n  --output_normalized                      Output Normalized.\n"
           );
    exit(exit_code);
}


int main(int argc, char *argv[])
{
    const char* program_name = argv[0];
    std::string tokenizer_type = "viterbi";
    std::string viterbi_word_dict = "viterbi_word.dict";
    std::string mfm_word_dict = "mfm_word.dict";
    bool output_normalized = false;

    const char* const short_options = "ht:m:v:n";
    const struct option long_options[] = {
        { "help",               0, NULL, 'h' },
        { "tokenizer_type",     1, NULL, 't' },
        { "mfm_word_dict",       1, NULL, 'm' },
        { "viterbi_word_dict",   1, NULL, 'v' },
        { "output_normalized",  0, NULL, 'n' },
        { NULL,                 0, NULL, 0   }
    };

    int next_option = -1;
    do {
        next_option = getopt_long (argc, argv, short_options,
                long_options, NULL);
        switch (next_option)
        {
            case 'h':
                print_usage(program_name, 0);
            case 't':
                tokenizer_type = optarg;
                break;
            case 'm':
                mfm_word_dict = optarg;
                break;
            case 'v':
                viterbi_word_dict = optarg;
                break;
            case 'n':
                output_normalized = true;
                break;
            case '?':
                print_usage(program_name, 1);
            case -1:
                break;
            default:
                abort ();
        }
    } while (next_option != -1);


    std::map<std::string, std::string> config_map;
    config_map["qws.tokenizer_type"] = tokenizer_type;
    config_map["qws.viterbi_word_dict"] = viterbi_word_dict;
    config_map["qws.mfm_word_dict"] = mfm_word_dict;
    if (output_normalized) {
        config_map["qws.output_normalized"] = "true";
    } else {
        config_map["qws.output_normalized"] = "false";
    }

    qcontent::QWSConfig config(config_map);
    qcontent::QWS qws;

    if (!qws.init(&config)) {
        fprintf(stderr, "init fatal");
        exit(1);
    }

    time_t start = time (NULL);
    int cnt = 0;
    std::string text;
    while (std::getline(std::cin, text)) {
        cnt++;
        qcontent::QWSTokenList tokens;
        qws.tokenize(text, tokens);
        int size = tokens.size();
        for (int i = 0; i < size; ++i) {
            if (i != size - 1) {
                std::cout << tokens[i].value << " ";
            } else {
                std::cout << tokens[i].value;
            }
        }
        std::cout << "\n";
    }
    time_t end = time (NULL);
    fprintf(stdout, "speed: %f\n",  (double)cnt / (end - start));

    return 0;
}
