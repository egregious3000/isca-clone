isca-clone
==========

Clone of ISCA BBS


To install:

make
make install
./bbs # make your first user who will be programmer, then logout
./bbs -c # this "clones" data, see below
make info # install forum information
./bbs -q # I do this inside a 'screen' so I can watch for error output


That's the theory.  I haven't done a completely clean install in a
while so maybe I missed something in /bbs from the "make install"
script.  "make install" does reset forums, users, and posts, though.

At the bottom of main.c you can see what functions "bbs -c" will call,
all inside isca.c.  Those routines are largely cut-and-pasted from
other files, and then modified.  They might not all be enabled
depending on what I was debugging at the time of my last push, but one
will probably want to test them out at a time when replicating the
work.

I don't have the scripts that suck out data checked in, because I'm
paranoid about being told I'm posting "dangerous" things.  I should
probably check in my intermediate files, though, so other people can
test and replicate my import logic.  But as a quick run-down, we
import:

1. FORUMS.  This just reads in (from "/tmp/forums") the list you get
by typing "LIST" in raccdoc.  If a Sysop runs it I think they should
see all private forums and invite-only forums, too, but I don't know
if there is any way to distinguish between those latter two in the
Raccdoc output.  I manually put in "Mail" and "Yells" at slots 1
and 2 in my list.

(I don't import the FM handle or the FI as of yet.  The FM user id is
readable from the above list so it should be easy to add.  Forum infos
go into their own file in /bbs/message/desc, and those are readable
over Raccdoc via the "show info" command.)

2. POSTS.  This reads in from a text file (from "tmp/posts") that are
line-oriented lists of header information followed by the post.  I put
an example at the bottom of this post.  I wrote a script that sucks
this all from Raccdoc to make this flat file, and it does take a
while, particularly because I don't want to be too punitive to either
end of the connection so I throttled it hard.

If the author cannot be found, it makes it be user #7 (which I think
is the old "Anonymous" user that ISCABBS had waaay back in the day,
and if I'm wrong on the number let me know).  There are two anon-type
posts and I can't tell the difference over Raccdoc, so I just assume
it's type 1 if forum=='weird' or type 2 if not.  The time of anon
posts is also unreadable from my account, but maybe Sysops/Programmers
can see the time over the Raccdoc interface.

This import function probably needs the closest analysis, because if I
put in a buffer-overflow anywhere, it's here.  I'd notice that
sometimes fgetc would return nulls for about a dozen characters when
the file pointer hit places like 32768, but I worked around that by
having a buffer line between posts. (At the limit, if we can't trust
this, then we can make another bbs command only visible to the
*Programmer* account that lets the operator post while setting the
post type, post time, and user id, and then script just call it via
telnet to populate the posts.  It will take longer but it's just a
one-time operation.)

3. USERS.  I've got a user profile script that I've been using for
over 10 years now to count users on the BBS, and this just reads in
the output of that (from "/tmp/profiles").  I modified the output to
add the "Anonymous" account at user #7 (is that right?) and Guest at
#6 (is my memory correct about that user number?).

Right now the only thing that is imported is the username, the user
number, and the TWIT/Sysop/Programmer flags (although only the
programmer can see the Programmer flag so I've never exercised that
code).  I force the number of posts made to be 51 to avoid the ugly
"are you sure you want to post?" noise.  And I made "Anonymous"
have an unusable password.  It will timeout anyway.

There is important user data simply not available via the P>rofile
command, even to users themselves or Programmers, like the current
message pointer in each forum, so lots of time spent parsing profiles
is probably wasted.  I'm working on an entirely different function
that reads the userdata file and spits that data out to a flat file.
Hopefully architecture differences will be transparent (since a
version running on HPUX will naturally read HPUX-formatted files).

Format for /tmp/posts file:


BUFFER BETWEEN POSTS:==============================================
FORUM: 0
POST: 2367
AUTHOR: 29431
DATE: 1314675420
SPECIAL: Sysop
BODY: 
We will soon be transferring the ISCABBS.COM domain name to a new
registrar.  Although we do not anticipate a service interruption
for any users, if you experience difficulty connecting to ISCA
over the coming days, it could be an unintended consequence of this
switch.  We have already transferred ISCABBS.ORG, so if you do have
trouble with ISCABBS.COM, you can connect through ISCABBS.ORG.

.

