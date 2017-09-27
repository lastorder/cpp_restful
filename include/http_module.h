#pragma once

#ifdef _WIN32
    #ifdef HTTPMODULE_EXPORTS
    #define HTTP_API __declspec(dllexport)
    #else
    #define HTTP_API __declspec(dllimport)
    #endif
#else
    #define HTTP_API
#endif


#include <map>
#include <functional>
#include <memory>
#include <vector>

using PairIntString = std::pair<int, std::string>;
using MapStringString = std::map<std::string, std::string>;
using HttpFuc = std::function<PairIntString(void)>;

struct http_server_fuc_struc
{
    int opt;
    std::string url;
    HttpFuc fuc;
};

#define HTTP_OPT_GET 1
#define HTTP_OPT_POST  2
#define HTTP_OPT_HEAD  4
#define HTTP_OPT_PUT  8
#define HTTP_OPT_DELETE  16


class http_server_if
{
public:
    virtual ~http_server_if() {};
    virtual void setTimeOut(const unsigned int timeout) = 0;
    virtual void registFuction(int opt, const std::string& url, HttpFuc fuc) = 0;
    virtual int start(const char* ip, const unsigned short port) = 0;
    virtual void setMaxWaitingRequest(const unsigned int) = 0;

    void registFuction(const std::vector<http_server_fuc_struc> fuclist)
    {
        for (auto& fuc: fuclist)
        {
            registFuction(fuc.opt, fuc.url, fuc.fuc);
        }
    };

};


class http_client_if
{
public:
    virtual void setTimeOut(const unsigned int timeout) = 0;
    virtual PairIntString request(int opt, const std::string& url, MapStringString params, const std::string & data) = 0;
    virtual ~http_client_if() {};
};



using LogFuc = std::function<void(int level,const char* msg)>;

HTTP_API int http_module_init(LogFuc fuc = NULL);
HTTP_API std::shared_ptr<http_server_if> http_creat_server(const unsigned int threads = 4);
HTTP_API std::shared_ptr<http_client_if> http_creat_client(const char* ip,unsigned short port);

// use in server callback function
HTTP_API const char* request_get_header(const char*);
HTTP_API const char* request_get_query(const char*);
HTTP_API const unsigned char* request_get_body();






