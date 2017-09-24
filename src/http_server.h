#pragma once
#include "http_module.h"
#include "ThreadPool.h"
#include <map>
#include <mutex>

struct PairIntStringCmp
{
    bool operator()(const PairIntString& ls, const PairIntString& rs)
    {
        return ls.first < rs.first || (ls.first == rs.first && ls.second < rs.second);
    }
};

using ServerFucMapValue = HttpFuc;
using ServerFucMap = std::map<PairIntString, ServerFucMapValue>;


class http_server;
static void hand_request(struct evhttp_request * req, http_server * serverPtr);
static void http_server_cb(struct evhttp_request *req, void *arg);

class http_server : public http_server_if
{

    friend void http_server_cb(struct evhttp_request *req, void *arg);
    friend void hand_request(struct evhttp_request * req, http_server * serverPtr);

public:
    http_server(const unsigned int threads);
    virtual ~http_server();
    virtual void setTimeOut(const unsigned int timeout);
    virtual void registFuction(int opt, const std::string& url, HttpFuc fuc);
    virtual int start(const char* ip, const unsigned short port);
    virtual void setMaxWaitingRequest(const unsigned int);

    ServerFucMapValue getFuction(int opt, const std::string& url);


private:
    unsigned int m_fixThreadCount;
    unsigned int m_timeout;
    ServerFucMap m_server_fuc_map;
    std::mutex m_fucMapMutex;
    ThreadPool m_pool;
    unsigned int m_maxQueue;
};

