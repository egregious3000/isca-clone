#  Makefile for ISCA BBS (HP-UX version for whip)
#
OPT=-O -g
DEFS=-D_GNU_SOURCE
#DEFS=-D_GNU_SOURCE -D_CONVERT_X86
#CFLAGS=-Wall -ansi
CFLAGS=-Wall -ansi -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wmissing-prototypes -Wnested-externs -Winline -Waggregate-return -Wundef -Wdeclaration-after-statement -Wendif-labels -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wold-style-definition -Wmissing-declarations -Wmissing-format-attribute -Wredundant-decls -Wpadded

SRCS= shell.c user.c system.c sysutil.c setup.c doc.c doc_msgs.c doc_rooms.c doc_routines.c xmsg.c doc_aide.c term.c state.c netio.c who.c finger.c users.c syncer.c update.c backup.c queue.c qmisc.c qrunbbs.c qstate.c utility.c main.c global.c linux.c endian.c
# new sources for ISCA-clone
SRCS+= isca.c
OBJS= $(SRCS:.c=.o)

.c.o:
	cc -c $< $(OPT) $(DEFS) $(CFLAGS) -g -ggdb

all:	bbs

bbs:	$(OBJS)
	cc -o bbs $(OBJS) -lcrypt -lcap -g -ggdb

clean:
	rm -f bbs $(OBJS)


$(OBJS): defs.h ext.h proto.h bbs.h telnet.h users.h queue.h qtelnet.h linux.h

install:
	mkdir -p /bbs
	rm -rf /bbs/data
	mkdir -p /bbs/data
	dd if=/dev/zero bs=35515 count=16   of=/bbs/data/tmpdata  2> /dev/null
	dd if=/dev/zero bs=4096 count=103   of=/bbs/data/msgdata  2> /dev/null
	dd if=/dev/zero bs=4096 count=103   of=/bbs/data/xmsgdata 2> /dev/null
	dd if=/dev/zero bs=4096 count=50000 of=/bbs/data/userdata 2> /dev/null
	rm -rf /bbs/message
	mkdir -p /bbs/message
	dd if=/dev/zero bs=4096 count=61036 of=/bbs/message/msgmain 2> /dev/null
	# need to init xmsgdata size, and make FI directory
