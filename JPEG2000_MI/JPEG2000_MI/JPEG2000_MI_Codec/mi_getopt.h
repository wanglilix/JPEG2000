

#ifndef _GETOPT_H_
#define _GETOPT_H_

typedef struct mi_option
{
	const char *name;
	int has_arg;
	int *flag;
	int val;
}mi_option_t;

#define	NO_ARG	0
#define REQ_ARG	1
#define OPT_ARG	2

extern int mi_opterr;
extern int mi_optind;
extern int mi_optopt;
extern int mi_optreset;
extern char *mi_optarg;

extern int mi_getopt(int nargc, char *const *nargv, const char *ostr);
extern int mi_getopt_long(int argc, char * const argv[], const char *optstring,
			const mi_option_t *longopts, int totlen);
extern void mi_reset_options_reading(void);

#endif				/* _GETOPT_H_ */
