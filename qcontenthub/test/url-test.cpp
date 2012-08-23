#include <msgpack/rpc/client.h>
#include <iostream>
#include <string>

#undef ASSERT
#define ASSERT(x) \
    if (! (x)) \
    { \
        cout << "ERROR!! Assert " << #x << " failed\n"; \
        cout << " on line " << __LINE__  << "\n"; \
        cout << " in file " << __FILE__ << "\n";  \
    }
#include "../qcontenthub.h"

using namespace std;

int main(void)
{
    int result;
    std::string stats;
    msgpack::rpc::client c("127.0.0.1", 19854);

    std::string site;
    std::string record;
    site = "site1";
    record = "r1";

    result = c.call("push", site, record).get<int>();
    std::cout << result << std::endl;

    record = "r2";
    result = c.call("push", site, record).get<int>();
    std::cout << result << std::endl;

    site = "site2";
    result = c.call("push", site, record).get<int>();
    std::cout << result << std::endl;

    stats = c.call("stats").get<std::string>();
    std::cout << stats << std::endl;
/*
    std::string pop;
    pop = c.call("pop").get<std::string>();
    std::cout << pop << std::endl;


    stats = c.call("stats").get<std::string>();
    std::cout << stats << std::endl;
*/
    result = c.call("start_dump_all").get<int>();
    std::cout << result << std::endl;

    int i = 0;
    while (stats != QCONTENTHUB_STREND) {
        stats = c.call("dump_all").get<std::string>();
        std::cout << "i : " << i++ << " " <<  stats << std::endl;
    }


    return 0;
}


