// tmain.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <fstream>

#include "../httpmodule/inc/http_module.h"
#include "../httpmodule/inc/log_base.h"

#include <thread>

static std::ofstream  logFile("logfile.txt");

void logimpl(int level, const char* msg)
{
    std::cout << msg;
    logFile << msg;
}

#define LOG_TRACE_D(msg) LOG_FORMAT_MSG(logimpl,0,"HTTP_MODULE",msg)
#define LOG_TRACE_E(msg) LOG_FORMAT_MSG(logimpl,4,"HTTP_MODULE",msg)


PairIntString test(const MapStringString& params, const std::string data)
{
    return PairIntString(200, "hello world test");
}

PairIntString sleep(const MapStringString& params, const std::string data)
{
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return PairIntString(200, "sleep for 10 secends !");
}

std::vector<http_server_fuc_struc> g_fuclist = {
    { HTTP_OPT_GET, "/test",test},
    { HTTP_OPT_POST,"/test",test },
    { HTTP_OPT_GET, "/sleep",sleep},
    { HTTP_OPT_POST,"/sleep",sleep }
};


#define IP "127.0.0.1"
#define PORT 8080

int main()
{
    LOG_TRACE_D("Hello world!");

    int ret = http_module_init(logimpl);

    auto serverPtr = http_creat_server(16);
    serverPtr->registFuction(g_fuclist);
    serverPtr->start(IP, PORT);

    std::chrono::seconds dura(2);
    std::this_thread::sleep_for(dura);

    

    auto testhand = [] {
        MapStringString params = { { "ABC","董宇" },{ "cd","test" } };
        auto clientPtr = http_creat_client(IP, PORT);
        auto resturn = clientPtr->request(HTTP_OPT_POST, "/test2", params, "hello world!");
        resturn = clientPtr->request(HTTP_OPT_POST, "/test", params, "hello world!");
        resturn = clientPtr->request(HTTP_OPT_POST, "/sleep", params, "hello world!");

    };
    int j = 10000;
    while (j--)
    {
        int i = 50;
        while (i--)
        {
            std::thread(testhand).detach();
        }

        std::chrono::seconds dura(30);
        std::this_thread::sleep_for(dura);
    }


    //_CrtDumpMemoryLeaks();
    system("Pause");
    return 0;
}

