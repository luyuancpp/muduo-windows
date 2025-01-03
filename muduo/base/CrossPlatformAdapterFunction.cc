﻿
#include <muduo/base/CrossPlatformAdapterFunction.h>

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include <muduo/base/Types.h>
#include "muduo/net/SocketsOps.h"

//////////////////////WIN32/////////////////////////////////
#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "ProcessInfo.h"
#endif // __linux__

#ifdef  WIN32

#include "Logging.h"

void setbuffer(FILE* stream, char* buf, size_t size)
{
    (void*)stream;
    (void*)buf;
    (void)size;
}

char* strerror_r(int errnum, char* buf, size_t buflen)
{
    strerror_s(buf, buflen, errnum);
    return buf;
}

size_t fwrite_unlocked(const void* ptr, size_t size, size_t n,
    FILE* stream)
{
    return ::fwrite(ptr, size, n, stream);
}

struct tm* gmtime_r(const time_t* timep, struct tm* result)
{
    gmtime_s(result, timep);
    return result;
}

int gettimeofday(struct timeval* tv, struct timezone* tz)
{
    (void)tz;
    std::micro microratio;
    uint64_t c = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    tv->tv_sec = (long)(c / microratio.den);
    tv->tv_usec = (long)(c % microratio.den);
    return 0;
}

int gethostbyname_r(const char* name,
    struct hostent* ret, char* buf, size_t buflen,
    struct hostent** result, int* h_errnop)
{
    return 0;
}

ssize_t pread(int fd, void* buf, size_t count, off_t offset)
{
    //return ::_read(fd, buf, count);
    (void)fd;
    (void*)buf;
    (void)count;
    (void)offset;
    return 0;
}

pid_t muduo::detail::gettid()
{
    return ::GetCurrentThreadId();
}

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
    return 0;
}

extern inline int __libc_use_alloca(size_t size)
{
    return size <= __MAX_ALLOCA_CUTOFF;
}

ssize_t winreadsock(int fd, void* buf, size_t count)
{
    WSAOVERLAPPED recvOverlapped = {};
    WSABUF dataBuf = {};
    DWORD recvBytes = 0;
    DWORD flags = 0;
    dataBuf.len = (ULONG)count;// notice bug
    dataBuf.buf = (char*)buf;

    int ret = WSARecv(fd, &dataBuf, 1, &recvBytes, &flags, &recvOverlapped, NULL);
    if (ret < 0)
    {
        printf("WSARecv failed with error: %d\n", WSAGetLastError());
        _set_errno(WSAGetLastError());
        return -1;
    }
    return recvBytes;
}

ssize_t winclosesock(int fd)
{
    return closesocket(fd);
}

//glibc
ssize_t readv(int fd, const struct iovec* vector, int count)
{

    /* Find the total number of bytes to be read.  */
    ssize_t bytes = 0;
    for (int i = 0; i < count; ++i)
    {
        /* Check for ssize_t overflow.  */
        if (SSIZE_MAX - bytes < vector[i].iov_len)
        {
            SetLastError(EINVAL);
            return -1;
        }
        bytes += vector[i].iov_len;
    }

    /* Allocate a temporary buffer to hold the data.  We should normally
    use alloca since it's faster and does not require synchronization
    with other threads.  But we cannot if the amount of memory
    required is too large.  */
    char* buffer = NULL;
    char* malloced_buffer = NULL;
    char stack_buffer[__MAX_ALLOCA_CUTOFF] = {};
    std::vector<char> mb((std::size_t)bytes);
    if (__libc_use_alloca((std::size_t)bytes))
        buffer = stack_buffer;
    else
    {
        malloced_buffer = buffer = mb.data();
    }

    /* Read the data.  */
    ssize_t bytes_read = ::winreadsock(fd, buffer, (std::size_t)bytes);
    if (bytes_read < 0)
        return -1;

    /* Copy the data from BUFFER into the memory specified by VECTOR.  */
    bytes = bytes_read;
    for (int i = 0; i < count; ++i)
    {
        ssize_t copy = std::min(vector[i].iov_len, bytes);

        (void)memcpy((void*)vector[i].iov_base, (void*)buffer, (std::size_t)copy);

        buffer += copy;
        bytes -= copy;
        if (bytes == 0)
            break;
    }

    return bytes_read;
}

void bzero(void* s, size_t n)
{
    memset(s, '\0', n);
}

char* basename(const char* full_path) {
    static char ret[1024];
    char path_copy[1024];
    char* ptr;

    if (full_path == NULL || strcmp(full_path, "") == 0) {
        strcpy_s(ret, ".");
    }
    else {
        strcpy_s(path_copy, full_path);
        char* p{ nullptr };
        ptr = strtok_s(path_copy, "/", &p);
        if (ptr == NULL)
            strcpy_s(ret, "/");

        while (ptr != NULL) {
            strcpy_s(ret, ptr);
            ptr = strtok_s(NULL, "/", &p);
        }
    }

    ptr = ret;
    return ptr;
}

const int64_t kNanoSecondsPerSecond = (int64_t)1e9;
int nanosleep(const struct timespec* req, struct timespec* rem)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(req->tv_sec * kNanoSecondsPerSecond + req->tv_nsec));
    (void)rem;
    return 0;
}

int setrlimit(int resource, const struct rlimit* rlim)
{
    (void)resource;
    (void)rlim;

    return 0;
}

class WindowsSocketInitFunction
{
public:
    WindowsSocketInitFunction()
    {
        WSADATA wsaData = { 0 };
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            //...
        }

    }
};


WindowsSocketInitFunction initSocketObj;




int timerfd_settime(int fd, int flags,
    const struct itimerspec* new_value,
    struct itimerspec* old_value)
{
    (void)fd;
    (void)flags;
    (void)new_value;
    (void)old_value;
    return 0;
}

unsigned int sleep(unsigned int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));

    return 0;
}
int poll(struct pollfd* fds, size_t nfds, int timeout)
{
    return WSAPoll(fds, (ULONG)nfds, timeout);
}

uid_t geteuid()
{
    return 0;
}


int getsockopt(int sockfd, int level, int optname,
    int* optval, socklen_t* optlen)
{
    return ::getsockopt(sockfd, level, optname, (char*)(optval), optlen);
}

void setsockopt(int sockfd, int level, int optname,
    int* optval, socklen_t optlen)
{
    ::setsockopt(sockfd, level, optname, (const char*)(optval), optlen);
}

int getsockopt(int sockfd, int level, int optname,
    tcp_info* optval, socklen_t* optlen)
{
    return ::getsockopt(sockfd, level, optname, (char*)(optval), optlen);
}

int accept4(int sockfd, struct sockaddr* addr,
    socklen_t* addrlen, int flags)
{
    int connfd = (int)::accept(sockfd, addr, addrlen);
    u_long  mode = 0;
    ioctlsocket(connfd, FIONBIO, &mode);
    return connfd;
}

int read(int _FileHandle, void* _DstBuf, size_t _MaxCharCount)
{
    return ::read(_FileHandle, _DstBuf, static_cast<uint32_t>(_MaxCharCount));
}


#pragma region strptime


//https://github.com/res2001/strptime/blob/master/strptime.c

/*	$NetBSD: strptime.c,v 1.62 2017/08/24 01:01:09 ginsbach Exp $	*/
/* http://cvsweb.netbsd.org/bsdweb.cgi/~checkout~/src/lib/libc/time/strptime.c?only_with_tag=HEAD
 * NetBSD implementation strptime().
 * Format description: https://netbsd.gw.com/cgi-bin/man-cgi?strptime+3+NetBSD-current
*/
/*-
 * Copyright (c) 1997, 1998, 2005, 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code was contributed to The NetBSD Foundation by Klaus Klein.
 * Heavily optimised by David Laight
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

 //#include <sys/cdefs.h>
 //__RCSID("$NetBSD: strptime.c,v 1.62 2017/08/24 01:01:09 ginsbach Exp $");

#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

static const unsigned char* conv_num(const unsigned char*, int*, unsigned int, unsigned int);
static const unsigned char* find_string(const unsigned char*, int*, const char* const*, const char* const*, int);

/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E			0x01
#define ALT_O			0x02
#define LEGAL_ALT(x)	{ if (alt_format & ~(x)) return NULL; }

#define TM_YEAR_BASE	1900

#define TM_SUNDAY       0
#define TM_MONDAY       1
#define TM_TUESDAY      2
#define TM_WEDNESDAY    3
#define TM_THURSDAY     4
#define TM_FRIDAY       5
#define TM_SATURDAY     6

#define S_YEAR			(1 << 0)
#define S_MON			(1 << 1)
#define S_YDAY			(1 << 2)
#define S_MDAY			(1 << 3)
#define S_WDAY			(1 << 4)
#define S_HOUR			(1 << 5)

#define HAVE_MDAY(s)	(s & S_MDAY)
#define HAVE_MON(s)		(s & S_MON)
#define HAVE_WDAY(s)	(s & S_WDAY)
#define HAVE_YDAY(s)	(s & S_YDAY)
#define HAVE_YEAR(s)	(s & S_YEAR)
#define HAVE_HOUR(s)	(s & S_HOUR)

#define SECSPERMIN      60
#define MINSPERHOUR     60
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define HOURSPERDAY     24

#define HERE_D_T_FMT    "%a %b %e %H:%M:%S %Y"
#define HERE_D_FMT      "%y/%m/%d"
#define HERE_T_FMT_AMPM "%I:%M:%S %p"
#define HERE_T_FMT      "%H:%M:%S"

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

 /*
 ** Since everything in isleap is modulo 400 (or a factor of 400), we know that
 **	isleap(y) == isleap(y % 400)
 ** and so
 **	isleap(a + b) == isleap((a + b) % 400)
 ** or
 **	isleap(a + b) == isleap(a % 400 + b % 400)
 ** This is true even if % means modulo rather than Fortran remainder
 ** (which is allowed by C89 but not by C99 or later).
 ** We use this to avoid addition overflow problems.
 */

#define isleap_sum(a, b)	isleap((a) % 400 + (b) % 400)

#ifdef _MSC_VER
#define tzname              _tzname
#define strncasecmp         _strnicmp
#endif

#ifdef TM_ZONE
static char* utc = "UTC";
#endif
/* RFC-822/RFC-2822 */
static const char* const nast[] = {
       "EST",    "CST",    "MST",    "PST",    "\0\0\0"
};
static const char* const nadt[] = {
       "EDT",    "CDT",    "MDT",    "PDT",    "\0\0\0"
};
static const char* weekday_name[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};
static const char* ab_weekday_name[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char* month_name[] =
{
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};
static const char* ab_month_name[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char* am_pm[] = { "AM", "PM" };


/*
 * Table to determine the ordinal date for the start of a month.
 * Ref: http://en.wikipedia.org/wiki/ISO_week_date
 */
static const int start_of_month[2][13] = {
    /* non-leap year */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* leap year */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

/*
 * Calculate the week day of the first day of a year. Valid for
 * the Gregorian calendar, which began Sept 14, 1752 in the UK
 * and its colonies. Ref:
 * http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
 */

static int
first_wday_of(int yr)
{
    return ((2 * (3 - (yr / 100) % 4)) + (yr % 100) + ((yr % 100) / 4) +
        (isleap(yr) ? 6 : 0) + 1) % 7;
}

#define delim(p)	((p) == '\0' || isspace((unsigned char)(p)))

static int
fromzone(const unsigned char** bp, struct tm* tm, int mandatory)
{
    //    timezone_t tz;
    char buf[512], * p;
    const unsigned char* rp;

    for (p = buf, rp = *bp; !delim(*rp) && p < &buf[sizeof(buf) - 1]; rp++)
        *p++ = *rp;
    *p = '\0';

    if (mandatory)
        *bp = rp;
    if (!isalnum((unsigned char)*buf))
        return 0;
    //    tz = tzalloc(buf);
    //    if (tz == NULL)
    //        return 0;

    *bp = rp;
    tm->tm_isdst = 0;	/* XXX */
#ifdef TM_GMTOFF
    tm->TM_GMTOFF = tzgetgmtoff(tz, tm->tm_isdst);
#endif
#ifdef TM_ZONE
    // Can't use tzgetname() here because we are going to free()
    tm->TM_ZONE = NULL; /* XXX */
#endif
    //    tzfree(tz);
    return 1;
}

char* strptime(const char* buf, const char* fmt, struct tm* tm)
{
    unsigned char c;
    const unsigned char* bp, * ep, * zname;
    int alt_format, i, split_year = 0, neg = 0, state = 0,
        day_offset = -1, week_offset = 0, offs, mandatory;
    const char* new_fmt;

    bp = (const unsigned char*)buf;

    while (bp != NULL && (c = *fmt++) != '\0') {
        /* Clear `alternate' modifier prior to new conversion. */
        alt_format = 0;
        i = 0;

        /* Eat up white-space. */
        if (isspace(c)) {
            while (isspace(*bp))
                bp++;
            continue;
        }

        if (c != '%')
            goto literal;


    again:		switch (c = *fmt++) {
    case '%':	/* "%%" is converted to "%". */
        literal :
            if (c != *bp++)
                return NULL;
        LEGAL_ALT(0);
        continue;

        /*
         * "Alternative" modifiers. Just set the appropriate flag
         * and start over again.
         */
    case 'E':	/* "%E?" alternative conversion modifier. */
        LEGAL_ALT(0);
        alt_format |= ALT_E;
        goto again;

    case 'O':	/* "%O?" alternative conversion modifier. */
        LEGAL_ALT(0);
        alt_format |= ALT_O;
        goto again;

        /*
         * "Complex" conversion rules, implemented through recursion.
         */
    case 'c':	/* Date and time, using the locale's format. */
        //            new_fmt = _TIME_LOCALE(loc)->d_t_fmt;
        new_fmt = HERE_D_T_FMT;
        state |= S_WDAY | S_MON | S_MDAY | S_YEAR;
        goto recurse;

    case 'F':	/* The date as "%Y-%m-%d". */
        new_fmt = "%Y-%m-%d";
        LEGAL_ALT(0);
        state |= S_MON | S_MDAY | S_YEAR;
        goto recurse;

    case 'R':	/* The time as "%H:%M". */
        new_fmt = "%H:%M";
        LEGAL_ALT(0);
        goto recurse;

    case 'r':	/* The time in 12-hour clock representation. */
        //            new_fmt = _TIME_LOCALE(loc)->t_fmt_ampm;
        new_fmt = HERE_T_FMT_AMPM;
        LEGAL_ALT(0);
        goto recurse;

    case 'X':	/* The time, using the locale's format. */
        /* fall through */

    case 'T':	/* The time as "%H:%M:%S". */
        new_fmt = HERE_T_FMT;
        LEGAL_ALT(0);

    recurse:
        bp = (const unsigned char*)strptime((const char*)bp,
            new_fmt, tm);
        LEGAL_ALT(ALT_E);
        continue;

    case 'x':	/* The date, using the locale's format. */
        /* fall throug */

    case 'D':	/* The date as "%y/%m/%d". */
    {
        new_fmt = HERE_D_FMT;
        LEGAL_ALT(0);
        state |= S_MON | S_MDAY | S_YEAR;
        const int year = split_year ? tm->tm_year : 0;

        bp = (const unsigned char*)strptime((const char*)bp,
            new_fmt, tm);
        LEGAL_ALT(ALT_E);
        tm->tm_year += year;
        if (split_year && tm->tm_year % (2000 - TM_YEAR_BASE) <= 68)
            tm->tm_year -= 2000 - TM_YEAR_BASE;
        split_year = 1;
        continue;
    }
    /*
     * "Elementary" conversion rules.
     */
    case 'A':	/* The day of week, using the locale's form. */
    case 'a':
        bp = find_string(bp, &tm->tm_wday, weekday_name, ab_weekday_name, 7);
        LEGAL_ALT(0);
        state |= S_WDAY;
        continue;

    case 'B':	/* The month, using the locale's form. */
    case 'b':
    case 'h':
        bp = find_string(bp, &tm->tm_mon, month_name, ab_month_name, 12);
        LEGAL_ALT(0);
        state |= S_MON;
        continue;

    case 'C':	/* The century number. */
        i = 20;
        bp = conv_num(bp, &i, 0, 99);

        i = i * 100 - TM_YEAR_BASE;
        if (split_year)
            i += tm->tm_year % 100;
        split_year = 1;
        tm->tm_year = i;
        LEGAL_ALT(ALT_E);
        state |= S_YEAR;
        continue;

    case 'd':	/* The day of month. */
    case 'e':
        bp = conv_num(bp, &tm->tm_mday, 1, 31);
        LEGAL_ALT(ALT_O);
        state |= S_MDAY;
        continue;

    case 'k':	/* The hour (24-hour clock representation). */
        LEGAL_ALT(0);
        /* FALLTHROUGH */
    case 'H':
        bp = conv_num(bp, &tm->tm_hour, 0, 23);
        LEGAL_ALT(ALT_O);
        state |= S_HOUR;
        continue;

    case 'l':	/* The hour (12-hour clock representation). */
        LEGAL_ALT(0);
        /* FALLTHROUGH */
    case 'I':
        bp = conv_num(bp, &tm->tm_hour, 1, 12);
        if (tm->tm_hour == 12)
            tm->tm_hour = 0;
        LEGAL_ALT(ALT_O);
        state |= S_HOUR;
        continue;

    case 'j':	/* The day of year. */
        i = 1;
        bp = conv_num(bp, &i, 1, 366);
        tm->tm_yday = i - 1;
        LEGAL_ALT(0);
        state |= S_YDAY;
        continue;

    case 'M':	/* The minute. */
        bp = conv_num(bp, &tm->tm_min, 0, 59);
        LEGAL_ALT(ALT_O);
        continue;

    case 'm':	/* The month. */
        i = 1;
        bp = conv_num(bp, &i, 1, 12);
        tm->tm_mon = i - 1;
        LEGAL_ALT(ALT_O);
        state |= S_MON;
        continue;

    case 'p':	/* The locale's equivalent of AM/PM. */
        bp = find_string(bp, &i, am_pm, NULL, 2);
        if (HAVE_HOUR(state) && tm->tm_hour > 11)
            return NULL;
        tm->tm_hour += i * 12;
        LEGAL_ALT(0);
        continue;

    case 'S':	/* The seconds. */
        bp = conv_num(bp, &tm->tm_sec, 0, 61);
        LEGAL_ALT(ALT_O);
        continue;

#ifndef TIME_MAX
#define TIME_MAX	INT64_MAX
#endif
    case 's':	/* seconds since the epoch */
    {
        time_t sse = 0;
        uint64_t rulim = TIME_MAX;

        if (*bp < '0' || *bp > '9') {
            bp = NULL;
            continue;
        }

        do {
            sse *= 10;
            sse += *bp++ - '0';
            rulim /= 10;
        } while ((sse * 10 <= TIME_MAX) &&
            rulim && *bp >= '0' && *bp <= '9');

        if (sse < 0 || (uint64_t)sse > TIME_MAX) {
            bp = NULL;
            continue;
        }
#ifdef _WIN32
        if (localtime_s(tm, &sse) == 0)
#else
        if (localtime_r(&sse, tm))
#endif
            state |= S_YDAY | S_WDAY | S_MON | S_MDAY | S_YEAR;
        else
            bp = NULL;
    }
    continue;

    case 'U':	/* The week of year, beginning on sunday. */
    case 'W':	/* The week of year, beginning on monday. */
        /*
         * This is bogus, as we can not assume any valid
         * information present in the tm structure at this
         * point to calculate a real value, so save the
         * week for now in case it can be used later.
         */
        bp = conv_num(bp, &i, 0, 53);
        LEGAL_ALT(ALT_O);
        if (c == 'U')
            day_offset = TM_SUNDAY;
        else
            day_offset = TM_MONDAY;
        week_offset = i;
        continue;

    case 'w':	/* The day of week, beginning on sunday. */
        bp = conv_num(bp, &tm->tm_wday, 0, 6);
        LEGAL_ALT(ALT_O);
        state |= S_WDAY;
        continue;

    case 'u':	/* The day of week, monday = 1. */
        bp = conv_num(bp, &i, 1, 7);
        tm->tm_wday = i % 7;
        LEGAL_ALT(ALT_O);
        state |= S_WDAY;
        continue;

    case 'g':	/* The year corresponding to the ISO week
             * number but without the century.
             */
        bp = conv_num(bp, &i, 0, 99);
        continue;

    case 'G':	/* The year corresponding to the ISO week
             * number with century.
             */
        do
            bp++;
        while (isdigit(*bp));
        continue;

    case 'V':	/* The ISO 8601:1988 week number as decimal */
        bp = conv_num(bp, &i, 0, 53);
        continue;

    case 'Y':	/* The year. */
        i = TM_YEAR_BASE;	/* just for data sanity... */
        bp = conv_num(bp, &i, 0, 9999);
        tm->tm_year = i - TM_YEAR_BASE;
        LEGAL_ALT(ALT_E);
        state |= S_YEAR;
        continue;

    case 'y':	/* The year within 100 years of the epoch. */
        /* LEGAL_ALT(ALT_E | ALT_O); */
        bp = conv_num(bp, &i, 0, 99);

        if (split_year)
            /* preserve century */
            i += (tm->tm_year / 100) * 100;
        else {
            split_year = 1;
            if (i <= 68)
                i = i + 2000 - TM_YEAR_BASE;
        }
        tm->tm_year = i;
        state |= S_YEAR;
        continue;

    case 'Z':       // time zone name
    case 'z':       //
#ifdef _WIN32
        _tzset();
#else
        tzset();
#endif
        mandatory = c == 'z';
        /*
         * We recognize all ISO 8601 formats:
         * Z	= Zulu time/UTC
         * [+-]hhmm
         * [+-]hh:mm
         * [+-]hh
         * We recognize all RFC-822/RFC-2822 formats:
         * UT|GMT
         *          North American : UTC offsets
         * E[DS]T = Eastern : -4 | -5
         * C[DS]T = Central : -5 | -6
         * M[DS]T = Mountain: -6 | -7
         * P[DS]T = Pacific : -7 | -8
         *          Nautical/Military
         * [A-IL-M] = -1 ... -9 (J not used)
         * [N-Y]  = +1 ... +12
         * Note: J maybe used to denote non-nautical
         *       local time
         */
        if (mandatory)
            while (isspace(*bp))
                bp++;

        zname = bp;
        switch (*bp++) {
        case 'G':
            if (*bp++ != 'M')
                goto namedzone;
            /*FALLTHROUGH*/
        case 'U':
            if (*bp++ != 'T')
                goto namedzone;
            else if (!delim(*bp) && *bp++ != 'C')
                goto namedzone;
            /*FALLTHROUGH*/
        case 'Z':
            if (!delim(*bp))
                goto namedzone;
            tm->tm_isdst = 0;
#ifdef TM_GMTOFF
            tm->TM_GMTOFF = 0;
#endif
#ifdef TM_ZONE
            tm->TM_ZONE = utc;
#endif
            continue;
        case '+':
            neg = 0;
            break;
        case '-':
            neg = 1;
            break;
        default:
        namedzone:
            bp = zname;

            /* Nautical / Military style */
            if (delim(bp[1]) &&
                ((*bp >= 'A' && *bp <= 'I') ||
                    (*bp >= 'L' && *bp <= 'Y'))) {
#ifdef TM_GMTOFF
                /* Argh! No 'J'! */
                if (*bp >= 'A' && *bp <= 'I')
                    tm->TM_GMTOFF =
                    (int)*bp - ('A' - 1);
                else if (*bp >= 'L' && *bp <= 'M')
                    tm->TM_GMTOFF = (int)*bp - 'A';
                else if (*bp >= 'N' && *bp <= 'Y')
                    tm->TM_GMTOFF = 'M' - (int)*bp;
                tm->TM_GMTOFF *= SECSPERHOUR;
#endif
#ifdef TM_ZONE
                tm->TM_ZONE = NULL; /* XXX */
#endif
                bp++;
                continue;
            }
            /* 'J' is local time */
            if (delim(bp[1]) && *bp == 'J') {
#ifdef TM_GMTOFF
                tm->TM_GMTOFF = -timezone;
#endif
#ifdef TM_ZONE
                tm->TM_ZONE = NULL; /* XXX */
#endif
                bp++;
                continue;
            }

            /*
             * From our 3 letter hard-coded table
             * XXX: Can be removed, handled by tzload()
             */
            if (delim(bp[0]) || delim(bp[1]) ||
                delim(bp[2]) || !delim(bp[3]))
                goto loadzone;
            ep = find_string(bp, &i, nast, NULL, 4);
            if (ep != NULL) {
#ifdef TM_GMTOFF
                tm->TM_GMTOFF = (-5 - i) * SECSPERHOUR;
#endif
#ifdef TM_ZONE
                tm->TM_ZONE = __UNCONST(nast[i]);
#endif
                bp = ep;
                continue;
            }
            ep = find_string(bp, &i, nadt, NULL, 4);
            if (ep != NULL) {
                tm->tm_isdst = 1;
#ifdef TM_GMTOFF
                tm->TM_GMTOFF = (-4 - i) * SECSPERHOUR;
#endif
#ifdef TM_ZONE
                tm->TM_ZONE = __UNCONST(nadt[i]);
#endif
                bp = ep;
                continue;
            }
            /*
             * Our current timezone
             */
            ep = find_string(bp, &i,
                (const char* const*)tzname,
                NULL, 2);
            if (ep != NULL) {
                tm->tm_isdst = i;
#ifdef TM_GMTOFF
                tm->TM_GMTOFF = -timezone;
#endif
#ifdef TM_ZONE
                tm->TM_ZONE = tzname[i];
#endif
                bp = ep;
                continue;
            }
        loadzone:
            /*
             * The hard way, load the zone!
             */
            if (fromzone(&bp, tm, mandatory))
                continue;
            goto out;
        }
        offs = 0;
        for (i = 0; i < 4; ) {
            if (isdigit(*bp)) {
                offs = offs * 10 + (*bp++ - '0');
                i++;
                continue;
            }
            if (i == 2 && *bp == ':') {
                bp++;
                continue;
            }
            break;
        }
        if (isdigit(*bp))
            goto out;
        switch (i) {
        case 2:
            offs *= SECSPERHOUR;
            break;
        case 4:
            i = offs % 100;
            offs /= 100;
            if (i >= SECSPERMIN)
                goto out;
            /* Convert minutes into decimal */
            offs = offs * SECSPERHOUR + i * SECSPERMIN;
            break;
        default:
        out:
            if (mandatory)
                return NULL;
            bp = zname;
            continue;
        }
        /* ISO 8601 & RFC 3339 limit to 23:59 max */
        if (offs >= (HOURSPERDAY * SECSPERHOUR))
            goto out;
        if (neg)
            offs = -offs;
        tm->tm_isdst = 0;	/* XXX */
#ifdef TM_GMTOFF
        tm->TM_GMTOFF = offs;
#endif
#ifdef TM_ZONE
        tm->TM_ZONE = NULL;	/* XXX */
#endif
        continue;

        /*
         * Miscellaneous conversions.
         */
    case 'n':	/* Any kind of white-space. */
    case 't':
        while (isspace(*bp))
            bp++;
        LEGAL_ALT(0);
        continue;


    default:	/* Unknown/unsupported conversion. */
        return NULL;
    }
    }

    if (!HAVE_YDAY(state) && HAVE_YEAR(state)) {
        if (HAVE_MON(state) && HAVE_MDAY(state)) {
            /* calculate day of year (ordinal date) */
            tm->tm_yday = start_of_month[isleap_sum(tm->tm_year,
                TM_YEAR_BASE)][tm->tm_mon] + (tm->tm_mday - 1);
            state |= S_YDAY;
        }
        else if (day_offset != -1) {
            /*
             * Set the date to the first Sunday (or Monday)
             * of the specified week of the year.
             */
            if (!HAVE_WDAY(state)) {
                tm->tm_wday = day_offset;
                state |= S_WDAY;
            }
            tm->tm_yday = (7 -
                first_wday_of(tm->tm_year + TM_YEAR_BASE) +
                day_offset) % 7 + (week_offset - 1) * 7 +
                tm->tm_wday - day_offset;
            state |= S_YDAY;
        }
    }

    if (HAVE_YDAY(state) && HAVE_YEAR(state)) {
        int isleap;

        if (!HAVE_MON(state)) {
            /* calculate month of day of year */
            i = 0;
            isleap = isleap_sum(tm->tm_year, TM_YEAR_BASE);
            while (tm->tm_yday >= start_of_month[isleap][i])
                i++;
            if (i > 12) {
                i = 1;
                tm->tm_yday -= start_of_month[isleap][12];
                tm->tm_year++;
            }
            tm->tm_mon = i - 1;
            state |= S_MON;
        }

        if (!HAVE_MDAY(state)) {
            /* calculate day of month */
            isleap = isleap_sum(tm->tm_year, TM_YEAR_BASE);
            tm->tm_mday = tm->tm_yday -
                start_of_month[isleap][tm->tm_mon] + 1;
            state |= S_MDAY;
        }

        if (!HAVE_WDAY(state)) {
            /* calculate day of week */
            i = 0;
            week_offset = first_wday_of(tm->tm_year);
            while (i++ <= tm->tm_yday) {
                if (week_offset++ >= 6)
                    week_offset = 0;
            }
            tm->tm_wday = week_offset;
            state |= S_WDAY;
        }
    }

    return (char*)bp;
}


static const unsigned char*
conv_num(const unsigned char* buf, int* dest, unsigned int llim, unsigned int ulim)
{
    unsigned int result = 0;
    unsigned char ch;

    /* The limit also determines the number of valid digits. */
    unsigned int rulim = ulim;

    ch = *buf;
    if (ch < '0' || ch > '9')
        return NULL;

    do {
        result *= 10;
        result += ch - '0';
        rulim /= 10;
        ch = *++buf;
    } while ((result <= ulim) && rulim && ch >= '0' && ch <= '9');

    if (result < llim || result > ulim)
        return NULL;

    *dest = result;
    return buf;
}

static const unsigned char*
find_string(const unsigned char* bp, int* tgt, const char* const* n1,
    const char* const* n2, int c)
{
    int i;
    size_t len;

    /* check full name - then abbreviated ones */
    for (; n1 != NULL; n1 = n2, n2 = NULL) {
        for (i = 0; i < c; i++, n1++) {
            len = strlen(*n1);
            if (strncasecmp(*n1, (const char*)bp, len) == 0) {
                *tgt = i;
                return bp + len;
            }
        }
    }

    /* Nothing matched */
    return NULL;
}
#pragma endregion 


#include <process.h>
#include <Windows.h>


namespace muduo
{
    namespace ProcessInfo
    {

        std::string hostname()
        {
            // HOST_NAME_MAX 64
            // _POSIX_HOST_NAME_MAX 255
            char buf[256];
            if (::gethostname(buf, sizeof buf) == 0)
            {
                buf[sizeof(buf) - 1] = '\0';
                return buf;
            }
            else
            {
                return "unknownhost";
            }
        }

    }

}//namespace muduo



namespace muduo
{
    namespace CurrentThread
    {

        string stackTrace(bool demangle)
        {
            string stack;
            const int len = 200;
            const int maxFunNameLen = 512;
            void* buffer[len];
            HANDLE         process = GetCurrentProcess();

            SymInitialize(process, nullptr, true);

            int nptrs = CaptureStackBackTrace(0, len, buffer, nullptr);

            SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + maxFunNameLen * sizeof(char), 1);
            symbol->MaxNameLen = maxFunNameLen;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

            if (symbol)
            {
                for (int i = 0; i < nptrs; ++i)
                {
                    SymFromAddr(process, (DWORD64)(buffer[i]), 0, symbol);
                    char funstring[maxFunNameLen] = {};
                    sprintf_s(funstring, "%i: %s - 0x%I64X\n", nptrs - i - 1, symbol->Name, symbol->Address);
                    stack.append(funstring);
                }
                free(symbol);
            }

            return stack;
        }

    }  // namespace CurrentThread
}  // namespace muduo


int muduo::net::sockets::createNonblockingOrDie(sa_family_t family)
{
    int sockfd = (int)::WSASocket(family, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sockfd < 0)
    {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    u_long  mode = 1;
    int result = ioctlsocket(sockfd, FIONBIO, &mode);
    if (result != NO_ERROR)
    {
        LOG_SYSFATAL << "ioctlsocket failed with error: " << result;
    }
    return sockfd;
}

int muduo::net::sockets::connect(int sockfd, const struct sockaddr* addr)
{
    auto result = ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (result == SOCKET_ERROR) {
        int error_code = WSAGetLastError();
        if (error_code == WSAEWOULDBLOCK) {
            LOG_INFO << "Connect is in progress (WSAEWOULDBLOCK). Waiting for completion...";

            // 使用 select 等待套接字可写
            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(sockfd, &writefds);

            timeval timeout;
            timeout.tv_sec = 5;  // 等待 5 秒
            timeout.tv_usec = 0;

            result = select(0, nullptr, &writefds, nullptr, &timeout);
            if (result > 0 && FD_ISSET(sockfd, &writefds)) {
                LOG_INFO << "Connection established!";
            }
            else if (result == 0) {
                LOG_INFO << "Connection timed out.";
            }
            else {
                LOG_ERROR << "Select failed with error: " << WSAGetLastError();
            }
            return 0;
        }
    }

    return result;
}

ssize_t muduo::net::sockets::read(int sockfd, void* buf, size_t count)
{
    return ::winreadsock(sockfd, buf, count);
}

ssize_t muduo::net::sockets::write(int sockfd, const void* buf, size_t count)
{
    WSAOVERLAPPED sendOverlapped = {};
    SecureZeroMemory((PVOID)&sendOverlapped, sizeof(WSAOVERLAPPED));


    DWORD sendBytes = 0;

    WSABUF dataBuf = {};
    dataBuf.len = (ULONG)count;//notice bug
    dataBuf.buf = (char*)buf;
    int ret = WSASend(sockfd, &dataBuf, 1, &sendBytes, 0, &sendOverlapped, NULL);
    if (ret < 0)
    {
        printf("WSASend failed with error: %d\n", WSAGetLastError());
        _set_errno(WSAGetLastError());
        return -1;
    }
    return sendBytes;
}

void muduo::net::sockets::close(int sockfd)
{
    if (::winclosesock(sockfd) < 0)
    {
        LOG_SYSERR << "sockets::close";
    }
}


#endif//WIN32


namespace muduo
{
    namespace ProcessInfo
    {
        std::string localip()
        {
            auto host = hostname();
            struct hostent* host_entry = gethostbyname(host.c_str());
            return inet_ntoa(*((struct in_addr*)host_entry->h_addr));
        }

    }

}//namespace muduo


void win_clear()
{
#ifdef WIN32
    WSACleanup();
#endif // WIN32
}

