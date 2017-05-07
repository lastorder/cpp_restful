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

using PairIntString = std::pair<int, std::string>;
using MapStringString = std::map<std::string, std::string>;
using HttpFuc = std::function<PairIntString(const MapStringString& params, const std::string& data)>;

struct http_server_fuc_struc
{
	int opt;
	std::string url;
	HttpFuc fuc;
	bool isSeparateThread;
};

#define HTTP_OPT_GET 1
#define HTTP_OPT_POST  2
#define HTTP_OPT_HEAD  4
#define HTTP_OPT_PUT  8
#define HTTP_OPT_DELETE  16


class http_server_if
{
public:

	virtual void setThreadCount(const unsigned int port) = 0;
	virtual void setTimeOut(const unsigned int timeout) = 0;
	virtual void registFuction(int opt, const std::string& url, HttpFuc fuc, bool isSeparateThread = false) = 0;
	virtual int start(const char* ip, const unsigned short port) = 0;

	void registFuction(const http_server_fuc_struc fuclist[], size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			registFuction(fuclist[i].opt, fuclist[i].url, fuclist[i].fuc, fuclist[i].isSeparateThread);
		}
	};

};


using LogFuc = std::function<void(int level,const char* msg)>;

HTTP_API int http_module_init(LogFuc fuc = NULL);
HTTP_API http_server_if* http_creat_server();




