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
    msgpack::rpc::client c("127.0.0.1", 9090);

    //c.set_timeout(1000);
    std::string queue_name = "crawler";
    stats = c.call("stat_queue", queue_name).get<std::string>();
    std::cout << stats << std::endl;
    ASSERT(stats.find(queue_name) != std::string::npos );

    std::string shift;
    shift = c.call("pop_nowait", queue_name).get<std::string>();
    std::cout << shift << std::endl;
    //ASSERT(shift == content);

    return 0;
}
