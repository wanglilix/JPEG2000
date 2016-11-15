
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)mi_getopt.c	8.3 (Berkeley) 4/27/95";
#endif				/* LIBC_SCCS and not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mi_getopt.h"

int mi_opterr = 1,			/* if error message should be printed */
 mi_optind = 1,			/* index into parent argv vector */
 mi_optopt,			/* character checked for validity */
 mi_optreset;			/* reset getopt */
 char *mi_optarg;			/* argument associated with option */

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
static char EMSG[]={""};

/* As this class remembers its values from one Java call to the other, reset the values before each use */
void mi_reset_options_reading(void) {
	mi_opterr = 1;
	mi_optind = 1;
}

/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int mi_getopt(int nargc, char *const *nargv, const char *ostr) {
#  define __progname nargv[0]
  static char *place = EMSG;	/* option letter processing */
  const char *oli = NULL;	/* option letter list index */

  if (mi_optreset || !*place) {	/* update scanning pointer */
    mi_optreset = 0;
    if (mi_optind >= nargc || *(place = nargv[mi_optind]) != '-') {
      place = EMSG;
      return (-1);
    }
    if (place[1] && *++place == '-') {	/* found "--" */
      ++mi_optind;
      place = EMSG;
      return (-1);
    }
  }				/* option letter okay? */
  if ((mi_optopt = (int) *place++) == (int) ':' ||
      !(oli = strchr(ostr, mi_optopt))) {
    /*
     * if the user didn't specify '-' as an option,
     * assume it means -1.
     */
    if (mi_optopt == (int) '-')
      return (-1);
    if (!*place)
      ++mi_optind;
		if (mi_opterr && *ostr != ':') {
      fprintf(stderr,
		     "%s: illegal option -- %c\n", __progname, mi_optopt);
			return (BADCH);
		}
  }
  if (*++oli != ':') {		/* don't need argument */
    mi_optarg = NULL;
    if (!*place)
      ++mi_optind;
  } else {			/* need an argument */
    if (*place)			/* no white space */
      mi_optarg = place;
    else if (nargc <= ++mi_optind) {	/* no arg */
      place = EMSG;
      if (*ostr == ':')
	return (BADARG);
			if (mi_opterr) {
				fprintf(stderr,
		       "%s: option requires an argument -- %c\n",
		       __progname, mi_optopt);
				return (BADCH);
			}
    } else			/* white space */
      mi_optarg = nargv[mi_optind];
    place = EMSG;
    ++mi_optind;
  }
  return (mi_optopt);		/* dump back option letter */
}


int mi_getopt_long(int argc, char * const argv[], const char *optstring,
const mi_option_t *longopts, int totlen) {
	static int lastidx,lastofs;
	const char *tmp;
	int i,len;
	char param = 1;

again:
	if (mi_optind >= argc || !argv[mi_optind] || *argv[mi_optind]!='-')
		return -1;

	if (argv[mi_optind][0]=='-' && argv[mi_optind][1]==0) {
		if(mi_optind >= (argc - 1)){ /* no more input parameters */
			param = 0;
		}
		else{ /* more input parameters */
			if(argv[mi_optind + 1][0] == '-'){
				param = 0; /* Missing parameter after '-' */
			}
			else{
				param = 2;
			}
		}
	}

	if (param == 0) {
		++mi_optind;
		return (BADCH);
	}

	if (argv[mi_optind][0]=='-') {	/* long option */
		char* arg=argv[mi_optind]+1;
		const mi_option_t* o;
		o=longopts;
		len=sizeof(longopts[0]);

		if (param > 1){
			arg = argv[mi_optind+1];
			mi_optind++;
		}
		else
			arg = argv[mi_optind]+1;

		if(strlen(arg)>1){
			for (i=0;i<totlen;i=i+len,o++) {
				if (!strcmp(o->name,arg)) {	/* match */
					if (o->has_arg == 0) {
						if ((argv[mi_optind+1])&&(!(argv[mi_optind+1][0]=='-'))){
							fprintf(stderr,"%s: option does not require an argument. Ignoring %s\n",arg,argv[mi_optind+1]);
							++mi_optind;
						}
					}else{ 
						mi_optarg=argv[mi_optind+1];
						if(mi_optarg){
							if (mi_optarg[0] == '-'){ /* Has read next input parameter: No arg for current parameter */								
								if (mi_opterr) {
									fprintf(stderr,"%s: option requires an argument\n",arg);
									return (BADCH);
								}
							}
						}
						if (!mi_optarg && o->has_arg==1) {	/* no argument there */
							if (mi_opterr) {
								fprintf(stderr,"%s: option requires an argument \n",arg);
								return (BADCH);
							}
						}
						++mi_optind;
					}
					++mi_optind;
					if (o->flag)
						*(o->flag)=o->val;
					else
						return o->val;
					return 0;
				}
			}/*(end for)String not found in the list*/
			fprintf(stderr,"Invalid option %s\n",arg);
			++mi_optind;
			return (BADCH);
		}else{ /*Single character input parameter*/
			if (*optstring==':') return ':';
			if (lastidx!=mi_optind) {
				lastidx=mi_optind; lastofs=0;
			}
			mi_optopt=argv[mi_optind][lastofs+1];
			if ((tmp=strchr(optstring,mi_optopt))) {/*Found input parameter in list*/
				if (*tmp==0) {	/* apparently, we looked for \0, i.e. end of argument */
					++mi_optind;
					goto again;
				}
				if (tmp[1]==':') {	/* argument expected */
					if (tmp[2]==':' || argv[mi_optind][lastofs+2]) {	/* "-foo", return "oo" as mi_optarg */
						if (!*(mi_optarg=argv[mi_optind]+lastofs+2)) mi_optarg=0;
						goto found;
					}
					mi_optarg=argv[mi_optind+1];
					if(mi_optarg){
						if (mi_optarg[0] == '-'){ /* Has read next input parameter: No arg for current parameter */
							if (mi_opterr) {
								fprintf(stderr,"%s: option requires an argument\n",arg);
								return (BADCH);
							}
						}
					}
					if (!mi_optarg) {	/* missing argument */
						if (mi_opterr) {
							fprintf(stderr,"%s: option requires an argument\n",arg);
							return (BADCH);
						}
					}
					++mi_optind;
				}else {/*Argument not expected*/
					++lastofs;
					return mi_optopt;
				}
found:
				++mi_optind;
				return mi_optopt;
			}	else {	/* not found */
				fprintf(stderr,"Invalid option %s\n",arg);
				++mi_optind;
				return (BADCH);
			}/*end of not found*/
		
		}/* end of single character*/
	}/*end '-'*/
	fprintf(stderr,"Invalid option\n");
	++mi_optind;
	return (BADCH);;
}/*end function*/
