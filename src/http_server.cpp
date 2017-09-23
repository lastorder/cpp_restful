#include "http_server.h"

#include "evhttp.h"
#include "event2/queue.h"

#include "http_module_log.h"
#include "scope_guard.h"
#include <thread>
#include <chrono>

extern std::string get_http_input_data(struct evhttp_request *req);

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

    struct evkeyval *query;
    TAILQ_FOREACH(query, &querys, next) {
        qmap[query->key] = query->value;
    }
}

static struct bufferevent * http_bev_cb(struct event_base *base, void *arg)
{
    return bufferevent_socket_new(
        base, -1,BEV_OPT_CLOSE_ON_FREE| BEV_OPT_CLOSE_ON_FREE| BEV_OPT_DEFER_CALLBACKS);
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
        LOG_TRACE_E("Not a good URI. Sending BADREQUEST !");
        evhttp_send_error(req, HTTP_BADREQUEST, "Sending BADREQUEST !");
        return;
    }

    const char* path = evhttp_uri_get_path(decoded);
    std::string strUrl = "/";
    if (path) strUrl = path;

    std::map<std::string, std::string> params;
    http_prase_query(evhttp_uri_get_query(decoded), params);

    if (NULL == arg)
    {
        evhttp_send_error(req, HTTP_INTERNAL, "Server ptr is NULL !");
        LOG_TRACE_E("Server ptr is NULL !");
        return;
    }
    auto request_data = get_http_input_data(req);
    if (!request_data.empty())
    {
        LOG_TRACE_D("request data : "<< request_data.c_str());
    }

    http_server* serverPtr = (http_server*)arg;

    if (serverPtr->m_pool.task_size() > 4* serverPtr->m_fixThreadCount)
    {
        evhttp_send_error(req, HTTP_SERVUNAVAIL, "Server is busy!");
        LOG_TRACE_D("Server is busy! ");
        return;
    }

    auto fuc = serverPtr->getFuction(opt, strUrl);

    if (nullptr == fuc)
    {
        evhttp_send_error(req, HTTP_NOTIMPLEMENTED, "Server path not implemented !");
        LOG_TRACE_E("Server path not implemented ! ");
        return;
    }
    else
    {
        //evhttp_request_own(req);
        auto hand = [=]() {
            struct evbuffer *evb = evbuffer_new();
            ON_SCOPE_EXIT([&] { if (evb) evbuffer_free(evb); });

            LOG_TRACE_D("Start hand " << strUrl);
            auto time_start = std::chrono::steady_clock::now();
            auto result = fuc(params, request_data);
            auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - time_start);
            LOG_TRACE_D("End hand " << strUrl <<" , Time use " << std::fixed << std::setprecision(3) << time_span.count());

            evbuffer_add_printf(evb, result.second.c_str());
            evhttp_send_reply(req, result.first, "OK", evb);
            //evhttp_request_free(req);
        };

        (void)serverPtr->m_pool.enqueue(hand);
        //std::thread(hand).detach();

    }

}

http_server::http_server(const unsigned int threads)
    :m_fixThreadCount(threads),
    m_timeout(600),
    m_fucMapMutex(),
    m_pool(threads)
{
}


http_server::~http_server()
{
}

void http_server::setTimeOut(const unsigned int timeout)
{
    m_timeout = timeout;

}

void http_server::registFuction(int opt, const std::string& url, HttpFuc fuc)
{
    std::lock_guard<std::mutex> grd(m_fucMapMutex);
    m_server_fuc_map[std::make_pair(opt, url)] = fuc;
}

int http_server::start(const char * ip, const unsigned short port)
{
    struct event_base *base = event_base_new();

    if (!base) {
        LOG_TRACE_E("Couldn't create an event_base: exiting");
        return 1;
    }

    /* Create a new evhttp object to handle requests. */
    struct evhttp *http = evhttp_new(base);
    if (!http) {
        LOG_TRACE_E("couldn't create evhttp. Exiting.");
        return 1;
    }

    struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, ip, port);
    if (!handle) {
        LOG_TRACE_E("couldn't bind to port : " << port);
        return 1;
    }
    evutil_socket_t fd = evhttp_bound_socket_get_fd(handle);
    if (evutil_make_socket_nonblocking(fd) != 0)
    {
        LOG_TRACE_E("evutil_make_socket_nonblocking failed!! ");
        return 1;
    }

    evhttp_set_timeout(http, m_timeout);
    evhttp_set_bevcb(http, http_bev_cb, this);
    evhttp_set_gencb(http, http_server_cb, this);
    std::thread(event_base_dispatch, base).detach();

    LOG_TRACE_D("Http server start ip " << ip <<":"<<port);
    return 0;
}

ServerFucMapValue http_server::getFuction(int opt, const std::string & url)
{
    std::lock_guard<std::mutex> grd(m_fucMapMutex);
    auto it = m_server_fuc_map.find(std::make_pair(opt, url));
    if (it != m_server_fuc_map.end())
    {
        return it->second;
    }
    return ServerFucMapValue();
}
