#include <mp/sync.h>
#include <mp/pthread.h>
#include <vector>
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


using namespace mp::placeholders;
using namespace std;

std::string queue_name = "test_queue";
int result;

void push_main(int a)
{
    msgpack::rpc::client c("127.0.0.1", 9090);
    c.set_timeout(1000000);
    for (int i = 0; i < 100; i++) {
        char buf[30];
        sprintf(buf, "%d", a);
        usleep(100);
        std::string content(buf);
        try {
            result = c.call("push", queue_name, content).get<int>();
            if (result != 0) {
                std::cout << result << std::endl;
            }
        } catch (std::exception& e) {
            std::cout << "push: " << std::string(e.what()) << std::endl;
            return;
        }
    }
    //ASSERT(result == 0);
}

void pop_main(int )
{
    msgpack::rpc::client c("127.0.0.1", 9090);
    c.set_timeout(1000000);
    for (int i = 0; i < 100; i++) {
        std::string shift;
        try {
            shift = c.call("pop", queue_name).get<std::string>();
            if (shift.find("error") != std::string::npos) {
                std::cout << shift << std::endl;
            }
        } catch (std::exception& e) {
            std::cout << "pop: " << std::string(e.what()) << std::endl;
            return;
        }
    }
}

int main(void)
{
    msgpack::rpc::client c("127.0.0.1", 9090);
    result = c.call("add", queue_name, 100).get<int>();
    std::cout << result << std::endl;

	std::vector<mp::pthread_thread> threads(50);
    for(int i=0; i < 25; ++i) {
		threads[i].run(mp::bind(&push_main, i));
	}
    for(int i=25; i < 50; ++i) {
		threads[i].run(mp::bind(&pop_main, i));
	}

	for(int i=0; i < 50; ++i) {
		threads[i].join();
	}
	std::cout << std::endl;
}
