/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: thread_pool.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_THREADING_THREAD_POOL_H
#define BASE_THREADING_THREAD_POOL_H

#include <vector>
#include <future>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <condition_variable>


#include "thread_safe_queue.h"
#include "function_wrapper.h"
#include "work_stealing_queue.h"

namespace base {

class ThreadPool {
 public:
    // 这个函数只能在你的进程中只会创建一个线程池时使用, 如果想要同时使用两个线程池来
    // 工作，请不要使用这个函数，并且定义PARALLEL_USE_MULIPLE_THREAD_POOL宏来禁止.
#ifndef PARALLEL_USE_MULIPLE_THREAD_POOL
	static ThreadPool* Current();
#endif

	explicit ThreadPool(std::size_t thread_num = 
						std::thread::hardware_concurrency());

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool operator=(const ThreadPool&) = delete;

    // 开启一个线程，这个可以重复调用，但是只能在running = false时才能调用，
    // 在调用了这个函数之后，想要将running设置为false，只有调用JoinAll，
    // 要想多次使用这个ThreadPool, 在调用了这个线程之后,请务必调用JoinAll.
	void Start();

	void JoinAll();

	template <typename Function>
	auto AddWork(Function f)
		->std::future<typename std::result_of<Function()>::type>;

	~ThreadPool();

 private:
	 using Task = base::FunctionWrapper;

	 void RunPendingTask();
	 void WorkerThread(unsigned int index);
	 bool PopTaskFromLocalQueue(Task& task) {
		 return local_work_queue_ && local_work_queue_->TryPop(task);
	 }

	 bool PopTaskFromPoolQueue(Task& task) {
		 return pool_work_queue_.TryPop(task);
	 }

	 bool PopTaskFromOtherThreadQueue(Task& task) {
		 const std::size_t size = queues_.size();
		 // 从index后面的线程队列开始偷取任务.
		 for (unsigned int i = 0; i < size; ++i) {
			 const unsigned int index = (index_ + i + 1) % size;
			 if (queues_[index_]->TrySteal(task))
				 return true;
		 }
		 return false;
	 }

	 std::atomic_bool running_ = false;
     // 所有线程池共享这个队列，每个线程都可以从这个队列取任务，和添加任务，但是
     // 这个从这个队列添加或者取出任务的优先级很低，比如，当前有一个线程想要从队列
     // 中拿任务执行，首先会从自己局部的queue中取任务(local_work_queue_), 如果队列
     // 中没有任务就会来这个队列中取任务.
	 base::ThreadSafeQueue<Task> pool_work_queue_;
     // 这个队列数组保存着每个线程的局部队列, 用于别的线程从其他线程窃取任务,比如在
     // pool_work_queue_中拿不到任务，就会从这个queues中取任务.
	 std::vector<std::unique_ptr<internal::WorkStealinggQueue>> queues_;
	 std::condition_variable cond_var_queues_;

	 std::size_t thread_num_;
	 std::vector<std::thread> threads_;
	 static thread_local internal::WorkStealinggQueue* local_work_queue_;
	 static thread_local unsigned int index_;
};



template<typename Function>
inline auto ThreadPool::AddWork(Function f) 
	-> std::future<typename std::result_of<Function()>::type> {
	typedef typename std::result_of<Function()>::type result_type;
	std::packaged_task<result_type()> task(f);
	std::future<result_type> res(task.get_future());

	if (local_work_queue_)
		local_work_queue_->Push(std::move(task));
	else
		pool_work_queue_.Push(std::move(task));

	return res;
}

// 一个非常简单的函数封装，只能使用于只有一个线程池的时候
#ifndef PARALLEL_USE_MULIPLE_THREAD_POOL
template <typename Fun, typename... Args>
inline auto PostTaskToThreadPool(Fun f, Args... args)
	->std::future<typename std::result_of<Fun(Args...)>::type> {
	return ThreadPool::Current()->AddWork(
		std::bind(std::forward<Fun>(f), std::forward<Args>(args)...));
}
#endif

}

#endif // !BASE_THREADING_THREAD_POOL_H
