#include <mutex>
#include <condition_variable>

using namespace std;

class barrier {

private:
    mutex mut;
    condition_variable cond;

    size_t num_threads;

public:
    explicit barrier(size_t num_threads) throw (std::invalid_argument);
    barrier():barrier(1){};
    ~barrier();

    void wait();
    void count_up();

    void reset(size_t num_threads);
};
