#pragma once
#include <mutex>
#include <event2/event.h>
#include "http_module.h"
class http_client : public http_client_if
{
public:
    http_client(const char* ip,unsigned short port);

    virtual PairIntString request(int opt, const std::string& url, MapStringString params, const std::string & data);
    virtual void setTimeOut(const unsigned int timeout);
    virtual ~http_client();

    struct event_base * m_eventbase;
    struct evhttp_connection * m_connection;
    int m_httpCode;
    std::string m_httpData;
private:
    std::mutex m_clientMutex;
};

