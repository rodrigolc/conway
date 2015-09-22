#include "barrier.hpp"
#include <mutex>
#include <condition_variable>

using namespace std;

barrier::barrier(size_t num_threads) throw (std::invalid_argument){
    if (num_threads == 0){
        throw std::invalid_argument("Can't create empty barrier");
    }
    this->num_threads = num_threads;
}
barrier::~barrier(){
}


void barrier::wait(){
    unique_lock<mutex> lk(mut);
    this->num_threads--;
    if( num_threads == 0 ){
        cond.notify_all();
    }else{
        cond.wait(lk, [this]{return num_threads == 0;});
    }
}

void barrier::reset(size_t num_threads){
    this->num_threads = num_threads;
}

void barrier::count_up(){
    unique_lock<mutex> lk(mut);
    this->num_threads++;
}
