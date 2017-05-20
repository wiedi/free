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
 * Copyright 2017 Sebastian Wiedenroth
 */

#ifndef _NICENUM_H
#define _NICENUM_H

/*
 * Temporary home of nicenum, the nice number formatter, until #640 is fixed
 */

#ifdef __cplusplus
extern "C" {
#endif

void nicenum(uint64_t num, char *buf);

#ifdef __cplusplus
}
#endif

#endif /* _NICENUM_H */
