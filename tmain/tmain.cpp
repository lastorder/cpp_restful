// tmain.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include "../httpmodule/inc/http_module.h"
#include "../httpmodule/inc/log_base.h"



void logimpl(int level, const char* msg)
{
	std::cout << msg << std::endl;
}

#define LOG_TRACE_D(msg) LOG_FORMAT_MSG(logimpl,0,"HTTP_MODULE",msg)
#define LOG_TRACE_E(msg) LOG_FORMAT_MSG(logimpl,4,"HTTP_MODULE",msg)


PairIntString test(const MapStringString& params, const std::string data)
{
	return PairIntString(200, "hello world");
}

struct http_server_fuc_struc g_fuclist[] = {
	{ 1, "/test", test, false},
	{ 2,"/test",test,false }
};


int main()
{
	LOG_TRACE_D("Hello world!");

	int ret = http_module_init(logimpl);

	auto serverPtr = http_creat_server();
	serverPtr->registFuction(g_fuclist,sizeof(g_fuclist)/sizeof(http_server_fuc_struc));
	serverPtr->start("127.0.0.1",9090);

	system("Pause");
    return 0;
}

