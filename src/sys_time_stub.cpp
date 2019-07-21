
#include <sys/time.h>
#include <kfw_rtc.hpp>

extern "C" {
	int _gettimeofday( struct timeval *tv, void *tzvp );
};

int _gettimeofday( struct timeval *tv, void *tzvp )
{
    uint64_t t = kfw::Rtc::get_time();
    tv->tv_sec = t / 1000;
    tv->tv_usec = 0;
    return 0;  // return non-zero for error
}

