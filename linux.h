/*
 * For the Linux port, the msemaphore structure and msem_* functions are kept
 * in the source "as is", and just modified to use advisory file locking of the
 * ranges a given semaphore used.
 */
#define MSEM_UNLOCKED 0
struct msemaphore
{
  char unused[16];
};


/*
 * Ugly hack to look into struct FILE.  I thought I was the only one abusing
 * stdio this badly, but when I did a little googling to figure out how to do
 * it using Linux's libc it turns out Tk, Ruby and others do this too, so now
 * I don't feel so bad!
 */
#define INPUT_LEFT() (stdin->_IO_read_ptr < stdin->_IO_read_end)


/*
 * Linux uses the stupid BSD signal behavior by default and automatically
 * restarts system calls.  What were they thinking?!  At least there's an easy
 * way around it.
 */
#define signal sysv_signal
