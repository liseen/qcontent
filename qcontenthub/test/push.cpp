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

using namespace std;

int main(void)
{
    int result;
    std::string stats;
    msgpack::rpc::client c("127.0.0.1", 7676);

    //c.set_timeout(1000);
    std::string queue_name = "test_queue";
    std::string content = "test_content";
    stats = c.call("stats").get<std::string>();
    std::cout << stats << std::endl;
//    ASSERT(stats.find(queue_name) != std::string::npos);

    stats = c.call("stat_queue", queue_name).get<std::string>();
    std::cout << stats << std::endl;
//    ASSERT(stats.find(queue_name) != std::string::npos );

    result = c.call("push", queue_name, content).get<int>();
    std::cout << result << std::endl;
    ASSERT(result == 0);
    /*
    std::string shift;
    shift = c.call("pop", queue_name).get<std::string>();
    std::cout << shift << std::endl;
    ASSERT(shift == content);
*/
    return 0;
}
