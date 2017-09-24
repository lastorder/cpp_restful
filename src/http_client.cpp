#include "http_client.h"
#include "http_module_log.h"
#include <event2/http.h>
#include <event2/buffer.h>

const unsigned char* get_http_input_data(struct evhttp_request *req)
{
    struct evbuffer *evbuf = evhttp_request_get_input_buffer(req);
    // NULL terminal buffer
    if (!evbuf)
    {
        LOG_TRACE_E("evbuffer is null ! ");
        return nullptr;
    }

    if (0 != evbuffer_add(evbuf, "", 1))
    {
        LOG_TRACE_E("evbuffer_add failed ! ");
        return nullptr;
    }
     
    return evbuffer_pullup(evbuf, -1);
}

static void http_client_cb(struct evhttp_request *req, void *arg)
{
    if (!arg)
    {
        LOG_TRACE_D("http client arg is null ! ");
        return;
    }

    if (!req) {
        LOG_TRACE_D("evhttp_request is null ! ");
    }

    http_client* clientptr = (http_client *)arg;
    clientptr->m_httpCode = evhttp_request_get_response_code(req);

    auto data = get_http_input_data(req);

    if (data)
    {
        clientptr->m_httpData = (char*)data;
    }

    event_base_loopexit(clientptr->m_eventbase, NULL);
}

http_client::http_client(const char * ip, unsigned short port)
    :m_clientMutex(),
    m_httpCode(HTTP_BADREQUEST),
    m_httpData("")
{
    m_eventbase = event_base_new();
    m_connection = evhttp_connection_base_new(m_eventbase,NULL,ip,port);
    evhttp_connection_set_timeout(m_connection, 600);
}

PairIntString http_client::request(int opt, const std::string & url, MapStringString params, const std::string & data)
{
    std::lock_guard<std::mutex> grd(m_clientMutex);
    auto t1 = std::chrono::steady_clock::now();
    m_httpCode = HTTP_BADREQUEST;
    m_httpData = "Client Error!";

    //formate query url
    auto req = evhttp_request_new(http_client_cb, this);
    std::string newUrl = url;
    if (!params.empty())
    {
        newUrl += "?";
        for (auto& query : params)
        {
            newUrl += query.first;
            newUrl += "=";
            char *s = evhttp_uriencode(query.second.c_str(), -1, 0);
            newUrl += s;
            free(s);
            newUrl += "&";
        }
        newUrl = newUrl.substr(0,newUrl.length()-1);
    }

    // add request data
    if (!data.empty())
    {
        auto output_buffer = evhttp_request_get_output_buffer(req);
        evbuffer_add(output_buffer, data.c_str(), data.length());
        auto output_headers = evhttp_request_get_output_headers(req);
        evhttp_add_header(output_headers, "Content-Length", std::to_string(data.length()).c_str());
    }

    //start request
    int ret = evhttp_make_request(m_connection, req,(evhttp_cmd_type)opt, newUrl.c_str());
    if (ret != 0) {
        LOG_TRACE_D("evhttp_make_request failed !");
    }
    else
    {
        event_base_dispatch(m_eventbase);
    }

    auto t2 = std::chrono::steady_clock::now();
    auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    LOG_TRACE_D("http client time use : " << std::fixed << std::setprecision(3) << time_span.count() << "  ret: " << m_httpCode);// << "  " << m_httpData.c_str());
    return PairIntString(m_httpCode, m_httpData);
}

void http_client::setTimeOut(const unsigned int timeout)
{
    evhttp_connection_set_timeout(m_connection, timeout);
}

http_client::~http_client()
{
    if (NULL != m_connection)
        evhttp_connection_free(m_connection);

    if (NULL != m_eventbase)
        event_base_free(m_eventbase);
}
