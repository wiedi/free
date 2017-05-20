/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

/*
 * Copyright 2015 Nexenta Systems, Inc. All rights reserved.
 * Copyright 2017 Sebastian Wiedenroth
 */

/* Based on Brendan Gregg's swapinfo */

#include <stdio.h>
#include <sys/sysinfo.h>
#include <kstat.h>
#include <sys/swap.h>
#include <zone.h>
#include <errno.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <locale.h>
#include <strings.h>

#include "nicenum.h"

static char *progname;

typedef struct free_info {
	uint64_t mem_total;
	uint64_t mem_used;
	uint64_t mem_free;
	uint64_t mem_locked;
	uint64_t mem_kernel;
	uint64_t mem_cached;
	uint64_t swap_total;
	uint64_t swap_used;
	uint64_t swap_free;
} free_info_t;


int
get_free_info(free_info_t *fi)
{
	struct anoninfo ai;
	kstat_ctl_t *kc = NULL;
	kstat_named_t *knp = NULL;

	bzero(fi, sizeof (free_info_t));

	uint64_t pagesize = getpagesize();
	uint64_t pages = sysconf(_SC_PHYS_PAGES);

	fi->mem_total = pagesize * pages;


	if ((kc = kstat_open()) == NULL) {
		fprintf(stderr, "kstat_open() failed\n");
		return (-1);
	}

	kstat_t *ks = kstat_lookup(kc, "unix", 0, "system_pages");
	if (ks == NULL)
		goto err;

	kstat_read(kc, ks, 0);
	knp = kstat_data_lookup(ks, "pp_kernel");
	if (knp == NULL)
		goto err;
	uint64_t pp_kernel = (uint64_t)knp->value.ui64 * pagesize;

	knp = kstat_data_lookup(ks, "pageslocked");
	if (knp == NULL)
		goto err;
	uint64_t pageslocked = (uint64_t)knp->value.ui64 * pagesize;

	zoneid_t zid = getzoneid();
	if (zid > 0) {
		/* local zone */
		ks = kstat_lookup(kc, "memory_cap", zid, NULL);
		if (ks == NULL)
			goto err;
		kstat_read(kc, ks, 0);

		knp = kstat_data_lookup(ks, "rss");
		if (knp == NULL)
			goto err;
		fi->mem_used = (uint64_t)knp->value.ui64;
		fi->mem_free = fi->mem_total - fi->mem_used;
	} else {
		/* global zone */
		knp = kstat_data_lookup(ks, "freemem");
		if (knp == NULL)
			goto err;
		fi->mem_free = (uint64_t)knp->value.ui64 * pagesize;

		knp = kstat_data_lookup(ks, "availrmem");
		if (knp == NULL)
			goto err;
		fi->mem_used = ((uint64_t)knp->value.ui64 * pagesize)
		    - fi->mem_free;
	}

	ks = kstat_lookup(kc, "zfs", 0, "arcstats");
	if (ks == NULL)
		goto err;
	kstat_read(kc, ks, 0);

	knp = kstat_data_lookup(ks, "size");
	if (knp == NULL)
		goto err;
	fi->mem_cached = (uint64_t)knp->value.ui64;

	kstat_close(kc);

	if (pp_kernel < pageslocked) {
		fi->mem_kernel = pp_kernel - fi->mem_cached;
		fi->mem_locked = pageslocked - pp_kernel;
	} else {
		fi->mem_kernel = pageslocked - fi->mem_cached;
		fi->mem_locked = 0;
	}

	if (swapctl(SC_AINFO, &ai) == -1)
		goto err;
	fi->swap_total = ai.ani_max * pagesize;
	fi->swap_used = ai.ani_resv * pagesize;
	fi->swap_free = (ai.ani_max - ai.ani_resv) * pagesize;

	return (0);

err:
	(void) kstat_close(kc);
	return (-1);
}

void
print_human_free_info(free_info_t *fi)
{
	char mem_total[6], mem_used[6], mem_free[6];
	char mem_locked[6], mem_kernel[6], mem_cached[6];
	char swap_total[6], swap_used[6], swap_free[6];

	nicenum(fi->mem_total, mem_total);
	nicenum(fi->mem_used, mem_used);
	nicenum(fi->mem_free, mem_free);
	nicenum(fi->mem_locked, mem_locked);
	nicenum(fi->mem_kernel, mem_kernel);
	nicenum(fi->mem_cached, mem_cached);
	nicenum(fi->swap_total, swap_total);
	nicenum(fi->swap_used, swap_used);
	nicenum(fi->swap_free, swap_free);

	printf("%16s %10s %10s %10s %10s %10s\n",
	    "total", "used", "free", "locked", "kernel", "cached");
	printf("Mem:  %10s %10s %10s %10s %10s %10s\n",
	    mem_total, mem_used, mem_free,
	    mem_locked, mem_kernel, mem_cached);
	printf("Swap: %10s %10s %10s\n", swap_total, swap_used, swap_free);
}

void
print_numeric_free_info(free_info_t *fi, int div)
{
	printf("%16s %10s %10s %10s %10s %10s\n",
	    "total", "used", "free", "locked", "kernel", "cached");
	printf("Mem:  %10llu %10llu %10llu %10llu %10llu %10llu\n",
	    fi->mem_total / div,
	    fi->mem_used / div,
	    fi->mem_free / div,
	    fi->mem_locked / div,
	    fi->mem_kernel / div,
	    fi->mem_cached / div);
	printf("Swap: %10llu %10llu %10llu\n",
	    fi->swap_total / div,
	    fi->swap_used / div,
	    fi->swap_free / div);
}

static void
usage(void)
{
	(void) fprintf(stderr, gettext(
	    "Usage: %s [-b|-k|-m|-g|-h]\n"
	    "Display the amount of free and used system memory\n"),
	    progname);
	exit(2);
}

int main(int argc, char *argv[]) {
	int c;
	int div = 1;
	free_info_t fi;
	boolean_t human = B_TRUE;

	(void) setlocale(LC_ALL, "");

#if !defined(TEXT_DOMAIN)
#define	TEXT_DOMAIN "SYS_TEST"
#endif
	(void) textdomain(TEXT_DOMAIN);

	progname = basename(argv[0]);
	while ((c = getopt(argc, argv, "hbkmg?")) != -1) {
		switch (c) {
			case 'h':
				human = B_TRUE;
				break;
			case 'b':
				div = 1;
				human = B_FALSE;
				break;
			case 'k':
				div = 1024;
				human = B_FALSE;
				break;
			case 'm':
				div = 1024 * 1024;
				human = B_FALSE;
				break;
			case 'g':
				div = 1024 * 1024 * 1024;
				human = B_FALSE;
				break;
			case '?': /* fallthrough */
			default:
				usage();
				exit(1);
		}
	}

	if (get_free_info(&fi) < 0) {
		perror("get_free_info");
		return (-1);
	}

	if (human) {
		print_human_free_info(&fi);
	} else {
		print_numeric_free_info(&fi, div);
	}

	return (0);
}
