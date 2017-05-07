#pragma once
#include "../inc/http_module.h"
#include <map>
#include <mutex>

struct PairIntStringCmp
{
    bool operator()(const PairIntString& ls, const PairIntString& rs)
    {
        return ls.first < rs.first || (ls.first == rs.first && ls.second < rs.second);
    }
};

using ServerFucMapValue = std::pair<HttpFuc, bool>;
using ServerFucMap = std::map<PairIntString, ServerFucMapValue>;

class http_server : public http_server_if
{
public:
    http_server();
    virtual ~http_server();

    virtual void setThreadCount(const unsigned int count);
    virtual void setTimeOut(const unsigned int timeout);
    virtual void registFuction(int opt, const std::string& url, HttpFuc fuc, bool isSeparateThread);
    virtual int start(const char* ip, const unsigned short port);
    void serverHand(struct evhttp_request *req,int opt, const std::string&  url, const MapStringString& params, const std::string& data);

private:
    unsigned int m_fixThreadCount;
    unsigned int m_timeout;
    ServerFucMap m_server_fuc_map;
    std::mutex m_fucMapMutex;
};

