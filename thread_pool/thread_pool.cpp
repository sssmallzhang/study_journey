#include <iostream>
#include <queue>
#include <pthread.h>
#include <cstdio>
#include <memory>
#include <unistd.h>

class Thread_Pool;

class Task{
    public:
        Task(): parameter_(0), is_valid_(false){} 
        Task(int para): parameter_(para), is_valid_(true){}
        void exe(){
            std::cout << this -> parameter_ << "\n";
        }
        bool is_valid(){
            return is_valid_;
        }
    private:
        int parameter_;
        bool is_valid_;

};

class worker{
    public:
        worker(Thread_Pool *owner): owner_(owner){
            int ret = pthread_create(&t_id_, NULL, worker_call_back, this);
        }
        static void *worker_call_back(void *arg);

        pthread_t get_t_id(){
            return t_id_;
        }

    private:
        pthread_t t_id_;
        Thread_Pool *owner_;
};


class Thread_Pool{
    public:
        Thread_Pool(int n): thread_num_(n), task_list_(), worker_list_(), on_off_(1){
            if(n <= 0) throw("create thread_pool_failed\n");
            pthread_mutex_init(&mutex_, NULL);
            pthread_cond_init(&cond_, NULL);
            for(int i = 0; i < n; i++){
                worker_list_.push( std::make_unique<worker>(this) );
            }
        }
        ~Thread_Pool(){
            pthread_mutex_lock(&mutex_);
            on_off_ = 0;
            pthread_mutex_unlock(&mutex_);
            pthread_cond_broadcast(&cond_);
            for(size_t i = 0; i < thread_num_; i++){
                pthread_join(worker_list_.front() -> get_t_id(), NULL);
            }
            pthread_mutex_destroy(&mutex_);
            pthread_cond_destroy(&cond_);
        }

        void push_task(int para){
            pthread_mutex_lock(&mutex_);
            task_list_.push(Task(para));
            std::cout << "push " << para << " success\n";
            pthread_cond_signal(&cond_);
            pthread_mutex_unlock(&mutex_);
        }
        
        Task get_task(){
            
            pthread_mutex_lock(&mutex_);
            while(task_list_.empty()){
                if(on_off_ == 0) break;
                pthread_cond_wait(&cond_, &mutex_);
            }
            if(on_off_ == 0){
                pthread_mutex_unlock(&mutex_);
                return Task();
            }
            auto t = task_list_.front();
            task_list_.pop();
            pthread_mutex_unlock(&mutex_);
            return t;
        }


    private:
        int thread_num_;
        std::queue<Task> task_list_;
        std::queue<std::unique_ptr<worker>> worker_list_;
        pthread_mutex_t mutex_;
        pthread_cond_t cond_;
        int on_off_;

};

void *worker::worker_call_back(void *arg){
    worker *this_worker = (worker*)arg;
    while(1){
        Task t = this_worker -> owner_ -> get_task();
        if(!t.is_valid()) break;
        t.exe();
    }
    return nullptr;
}


int main(){
    Thread_Pool pool(10);
    std::cout << "create success" << "\n";
    int n = 20;
    while(n--){
        pool.push_task(n);
    }
    sleep(10);
}