#include "../inc/http_module.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#pragma comment(lib, "WS2_32") 
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include "http_module_log.h"
#include "http_server.h"
#include "http_client.h"
#include "event2\thread.h "

LogFuc g_logfuc = NULL;
HTTP_API int http_module_init(LogFuc fuc)
{
    g_logfuc = fuc;
    LOG_TRACE_D("http_module_init !");
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        LOG_TRACE_D("WSAStartup failed with error");
        return (1);
    }

    if (0 == evthread_use_windows_threads())
    {
        LOG_TRACE_D("evthread_use_windows_threads success !");
    }
    else
    {
        LOG_TRACE_D("evthread_use_windows_threads failed !");
        return 1;
    }
#else
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        LOG_TRACE_D("SIG_IGN failed with error");
        return (1);
    }
    if (0 == evthread_use_pthreads())
    {
        LOG_TRACE_D("evthread_use_pthreads success !");
    }
    else
    {
        LOG_TRACE_D("evthread_use_pthreads failed !");
        return 1;
    }
#endif
    return 0;
}

HTTP_API http_server_if* http_creat_server(const unsigned int threads)
{
    return new http_server(threads);
}

HTTP_API http_client_if * http_creat_client(const char * ip, unsigned short port)
{
    return new http_client(ip,port);
}

