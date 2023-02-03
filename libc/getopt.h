#ifndef _GETOPT_H
#define _GETOPT_H 1

extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

extern int getopt (int ___argc, char **___argv, const char *__shortopts);

struct option
{
  char *name;
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
};

#define no_argument		0
#define required_argument	1
#define optional_argument	2

extern int getopt_long (int ___argc, char **___argv,
			char *__shortopts, struct option *__longopts, int *__longind);

#endif