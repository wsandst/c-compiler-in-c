#include "args.h"

char* isolate_file_dir(char* filepath) {
    int index = str_index_of_reverse(filepath, '/');
    return str_substr(filepath, index + 1);
}

char* isolate_file_from_path(char* filepath) {
    int index = str_index_of_reverse(filepath, '/');
    return str_copy(filepath + index + 1);
}

// Parse commandline options using getopt
CompileOptions parse_compiler_options(int argc, char** argv) {
    opterr = 0;
    CompileOptions options;
    options.link_with_gcc = true;
    options.output_filename = "a.out";
    options.debug_annotate_assembly = false;
    options.keep_assembly = false;
    bool output_file_set = false;
    int option_index = 0;
    struct option long_options[25];
    long_options[0].name = "outfile";
    long_options[0].has_arg = required_argument;
    long_options[0].flag = 0;
    long_options[0].val = 'o';

    long_options[1].name = "nolink";
    long_options[1].has_arg = no_argument;
    long_options[1].flag = 0;
    long_options[1].val = 'c';

    long_options[2].name = "debug";
    long_options[2].has_arg = no_argument;
    long_options[2].flag = 0;
    long_options[2].val = 'g';

    long_options[3].name = "keepasm";
    long_options[3].has_arg = no_argument;
    long_options[3].flag = (int*)0;
    long_options[3].val = 'k';

    long_options[4].name = 0;
    long_options[4].has_arg = 0;
    long_options[4].flag = 0;
    long_options[4].val = 0;

    int opt_c = getopt_long(argc, argv, ":cgko:", long_options,
                            &option_index);
    // Get command line flags
    while (opt_c != -1) {
        switch (opt_c) {
            case 'o':
                options.output_filename = optarg;
                output_file_set = true;
                break;
            case 'c':
                options.link_with_gcc = false;
                break;
            case 'g':
                options.debug_annotate_assembly = true;
                break;
            case 'k':
                options.keep_assembly = true;
                break;
            case '?':
                if (optopt == 'o') {
                    fprintf(stderr, "Error: Option '-%c' requires a file argument\n", optopt);
                }
                else {
                    fprintf(stderr, "Error: Unknown option '-%c' provided\n", optopt);
                }
                fprintf(stderr, "Usage: ./ccic [-c] [-o FILENAME] [-g] [--keepasm] <FILE> [FILES ...]\n");
                exit(EXIT_FAILURE);
            default:
                exit(EXIT_FAILURE);
        }
        opt_c = getopt_long(argc, argv, ":cgko:", long_options,
                    &option_index);
    }
    // Isolate files to compile
    if (optind < argc) {
        options.src_filename = argv[optind];
        if (!output_file_set && !options.link_with_gcc) {
            // If an output filename was not given and we are linking, use
            // the source file as output name
            options.output_filename = isolate_file_from_path(options.src_filename);
            options.output_filename[strlen(options.output_filename) - 1] = 'o';
        }
        else {
            options.output_filename = str_copy(options.output_filename);
        }
    }
    else {
        fprintf(stderr, "Error: Please specify a source file to compile.\n");
        fprintf(stderr, "Usage: ./ccic [-c] [-o FILENAME] [-g] [--keepasm] <FILE> [FILES ...]\n");
        exit(EXIT_FAILURE);
    }
    return options;
}