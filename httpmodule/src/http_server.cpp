#include "http_server.h"

#include "evhttp.h"
#include "event2\queue.h"

#include "http_module_log.h"
#include "..\inc\scope_guard.h"
#include <thread>
#include <chrono>

static void http_prase_query(const char* qstr, std::map<std::string, std::string>& qmap)
{
    if (NULL == qstr)
    {
        return;
    }
    struct evkeyvalq querys;
    TAILQ_INIT(&querys);
    ON_SCOPE_EXIT([&] { evhttp_clear_headers(&querys); });

    if (0 != evhttp_parse_query_str(qstr, &querys))
    {
        LOG_TRACE_D("evhttp_parse_query_str failed!! ");
        return;
    }
    evhttp_parse_query_str(qstr,&querys);

    struct evkeyval *query;
    TAILQ_FOREACH(query, &querys, next) {
        qmap[query->key] = query->value;
    }
}

static void http_server_cb(struct evhttp_request *req, void *arg)
{
    const char *uri = evhttp_request_get_uri(req);
    evhttp_cmd_type opt = evhttp_request_get_command(req);

    LOG_TRACE_D("Got a [" << opt<<"] request for "<< uri);

    /* Decode the URI */
    struct evhttp_uri *decoded = evhttp_uri_parse(uri);
    ON_SCOPE_EXIT([&] { if (decoded) evhttp_uri_free(decoded); });

    if (!decoded) {
        LOG_TRACE_D("Not a good URI. Sending BADREQUEST !");
        evhttp_send_error(req, HTTP_BADREQUEST, "Sending BADREQUEST !");
        return;
    }

    /* Let's see what path the user asked for. */
    const char* path = evhttp_uri_get_path(decoded);
    if (!path) path = "/";

    std::map<std::string, std::string> params;
    http_prase_query(evhttp_uri_get_query(decoded), params);

    if (NULL == arg)
    {
        evhttp_send_error(req, HTTP_INTERNAL, "Server ptr is NULL !");
    }

    http_server* serverPtr = (http_server*)arg;
    serverPtr->serverHand(req,opt, path, params, "");

}

http_server::http_server()
    :m_fixThreadCount(2),
    m_timeout(600),
    m_fucMapMutex()
{
}


http_server::~http_server()
{
}

void http_server::setThreadCount(const unsigned int count)
{
    m_fixThreadCount = count;
}

void http_server::setTimeOut(const unsigned int timeout)
{
    m_timeout = timeout;

}

void http_server::registFuction(int opt, const std::string& url, HttpFuc fuc, bool isSeparateThread)
{
    std::lock_guard<std::mutex> grd(m_fucMapMutex);
    m_server_fuc_map[std::make_pair(opt, url)] = std::make_pair(fuc, isSeparateThread);
}

int http_server::start(const char * ip, const unsigned short port)
{
    evutil_socket_t fd = NULL;

    for (unsigned int i = 0; i < m_fixThreadCount; i++)
    {
        struct event_base *base = event_base_new();

        if (!base) {
            fprintf(stderr, "Couldn't create an event_base: exiting\n");
            return 1;
        }

        /* Create a new evhttp object to handle requests. */
        struct evhttp *http = evhttp_new(base);
        if (!http) {
            fprintf(stderr, "couldn't create evhttp. Exiting.\n");
            return 1;
        }

        if (NULL == fd)
        {
            struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, ip, port);
            if (!handle) {
                fprintf(stderr, "couldn't bind to port %d. Exiting.\n", (int)port);
                return 1;
            }
            fd = evhttp_bound_socket_get_fd(handle);
            if (evutil_make_socket_nonblocking(fd) != 0)
            {
                LOG_TRACE_D("evutil_make_socket_nonblocking failed!! ");
                return 1;
            }
        }
        else
        {
            if (0 != evhttp_accept_socket(http, fd))
            {
                LOG_TRACE_D("evhttp_accept_socket failed!! ");
                return 1;
            }
        }

        evhttp_set_timeout(http, m_timeout);
        evhttp_set_gencb(http, http_server_cb, this);
        std::thread(event_base_dispatch, base).detach();
    }
    LOG_TRACE_D("Http server start ip " << ip <<":"<<port);
    return 0;
}

void http_server::serverHand(struct evhttp_request *req,int opt, const std::string & url, const MapStringString & params, const std::string & data)
{
    HttpFuc fuc = NULL;
    bool isSThread = false;

    {
        std::lock_guard<std::mutex> grd(m_fucMapMutex);
        auto it = m_server_fuc_map.find(std::make_pair(opt, url));
        if (it != m_server_fuc_map.end())
        {
            fuc = it->second.first;
            isSThread = it->second.second;
        }
    }

    if (NULL == fuc)
    {
        evhttp_send_error(req, HTTP_NOTIMPLEMENTED, "Server path not implemented !");

        LOG_TRACE_D("Server path not implemented ! ");
        return;
    }

    auto hand = [&]() {
        struct evbuffer *evb = evb = evbuffer_new();
        ON_SCOPE_EXIT([&] { if (evb) evbuffer_free(evb); });

        auto t1 = std::chrono::steady_clock::now();
        auto result = fuc(params, data);
        auto t2 = std::chrono::steady_clock::now();
        auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        LOG_TRACE_D("Time use : " << std::fixed << std::setprecision(3) << time_span.count() << "  url: " << url);

        evbuffer_add_printf(evb, result.second.c_str());
        evhttp_send_reply(req, result.first, "OK", evb);

    };

    if (isSThread)
    {
        std::thread(hand).detach();
    }
    else
    {
        hand();
    }
}
