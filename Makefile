#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"), version 1.0.
# You may only use this file in accordance with the terms of version
# 1.0 of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source.  A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#

#
# Copyright 2015 Nexenta Systems, Inc. All rights reserved.
#

CC = gcc

CFLAGS= -g -O2 -Wall
LIBS= -lkstat
TARGET= free
OBJS= nicenum.o
SRCS= $(OBJS:.o=.c)

all: $(TARGET)

$(TARGET): $(OBJS) $(TARGET).c
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(TARGET).c $(LIBS)

$(OBJS): $(SRCS)
	$(COMPILE.c) $(SRCS)

clean:
	$(RM) $(TARGET) $(OBJS)
