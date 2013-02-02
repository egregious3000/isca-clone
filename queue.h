/*
 * Configuration options for queue
 */
#define LIMITFILE	"/bbs/etc/limits"
#define HELLOFILE	"/bbs/etc/hello"
#define DOWNFILE	"/bbs/etc/down"
#define PORT		23
#define MAXACTIVITY	60
#define LOCAL(x)	((((x) >> 16) & ~0x0100) == 0x80ff)

#define DUMPFILE	"/bbs/var/queuedump"

#define BBSEXEC		"/bbs/bin/bbs"
#define BBSARG		"_netbbs"
#define CLIENTARG	"_clientbbs"


/*
 * Strings for authorization subsystem
 */
#define ERASE		"\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b"

#define	INCORRECT	"\r\nIncorrect login.\r\n"

#define LOGGEDIN	"\r\nLogged in.\r\n\n"

#define AIDELOGGEDIN	"\r\nLogged in as administrator.\r\n\n"

#define HASONEMAIL	"You have 1 Mail> message.\r\n\n"

#define HASMANYMAIL	"You have %d Mail> messages.\r\n\n"


/*
 * Various informational strings for queue
 */
#define BBSFULL		"\r\nThe ISCA BBS is full at the moment.\r\n\n"

#define ATFRONT		"%s (%ld:%02ld) You are at the front of the queue\r\n"

#define ONEAHEAD	"%s (%ld:%02ld) There is 1 user queued ahead of you (%ld users %d queued)\r\n"

#define MANYAHEAD	"%s (%ld:%02ld) There are %d users queued ahead of you (%ld users %d queued)\r\n"

#define TOOMANY		"\007\r\n\nHitting keys inside the queue only serves to increase network traffic and slow\r\ndown the system for everyone.  You can escape the queue using ctrl-C, ctrl-D,\r\nor ctrl-Z.  Please use one of those keys instead.\r\n\n"

#define BBSGONE		"\007\r\n\nThe BBS cannot be started due to technical problems.  Please try again later.\r\n\n"

#define NEWUSERCREATE	"\r\n\nNew users cannot be created until you have finished the queue and entered the\r\nBBS or are a sysop.\r\n\nName: "

#define IFYOUHADCLIENT	"(If you had the BBS client you would have started with only %d users queued\r\nahead of you!  See the Client> forum for more information.)\r\n\n"
