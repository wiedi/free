/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 */


/*
 * nicenum taken from usr/src/lib/libzpool/common/util.c
 * should fix #640 and #5827 first and remove here
 */

#include <stdio.h>
#include <unistd.h>


void
nicenum(uint64_t num, char *buf)
{
	uint64_t n = num;
	int index = 0;
	char u;

	while (n >= 1024) {
		n = (n + (1024 / 2)) / 1024; /* Round up or down */
		index++;
	}

	u = " KMGTPE"[index];

	if (index == 0) {
		(void) sprintf(buf, "%llu", (u_longlong_t)n);
	} else if (n < 10 && (num & (num - 1)) != 0) {
		(void) sprintf(buf, "%.2f%c",
		    (double)num / (1ULL << 10 * index), u);
	} else if (n < 100 && (num & (num - 1)) != 0) {
		(void) sprintf(buf, "%.1f%c",
		    (double)num / (1ULL << 10 * index), u);
	} else {
		(void) sprintf(buf, "%llu%c", (u_longlong_t)n, u);
	}
}
