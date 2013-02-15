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
        # set xmsgdata size, we want "0000 0c35" in that location
	/bin/echo -ne "\x00\x00\x35\x0c"| dd of=/bbs/data/msgdata bs=16 seek=9  conv=notrunc
	dd if=/dev/zero bs=4096 count=50000 of=/bbs/data/xmsgdata 2> /dev/null
	dd if=/dev/zero bs=4096 count=50000 of=/bbs/data/userdata 2> /dev/null
	rm -rf /bbs/message
	mkdir -p /bbs/message
	dd if=/dev/zero bs=4096 count=61036 of=/bbs/message/msgmain 2> /dev/null
	mkdir -p /bbs/message/desc
	rm -rf /bbs/help
	mkdir -p /bbs/help
	cp data/new.* /bbs/help
	cp data/yell.* /bbs/help
	cp data/guestwelcome /bbs/help
	chown -R bbs /bbs
	cp data/list data/profiles /tmp
	cat data/posts-* > /tmp/posts

NEWLINE_SRCS=doc_aide.c doc.c doc_msgs.c doc_rooms.c doc_routines.c main.c setup.c shell.c state.c system.c sysutil.c term.c update.c user.c users.c who.c xmsg.c


# \n to \r\n
n2rn:
        # replace "\n" that is not following \r nor followed by '
	perl -pi \
	   -e "s/putchar\('\\\\n'\);/puts\(\"\\\\r\"\);/g" \
	${NEWLINE_SRCS}

	perl -pi \
	   -e "s/(?<!\\\\r)\\\\n(?!')/\\\\r\\\\n/g" \
        ${NEWLINE_SRCS}

# \r\n to \n
rn2n:
	perl -pi.bak \
	   -e "s/puts\(\"\\\\r\"\);/putchar\('\\\\n'\);/g" \
        ${NEWLINE_SRCS}

	perl -pi.bak \
           -e "s/\\\\r\\\\n/\\\\n/g" \
        ${NEWLINE_SRCS}


