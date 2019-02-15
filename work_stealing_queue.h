/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: thread_pool.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_WORK_STEALING_QUEUE_H
#define BASE_WORK_STEALING_QUEUE_H

#include <deque>
#include <mutex>

#include "function_wrapper.h"

namespace base {

namespace internal {

// 这是一个将std::dequeue结构封装成线程安全的队列，用来给ThreadPool使用
class WorkStealinggQueue {
	 using Data = base::FunctionWrapper;
 public:
	 WorkStealinggQueue() = default;

	 void Push(Data data) {
		 std::lock_guard<std::mutex> lock(mutex_);
		 queue_.push_front(std::move(data));
	 }

	 bool Empty() const {
		 std::lock_guard<std::mutex> lock(mutex_);
		 queue_.empty();
	 }

	 bool TryPop(Data& res) {
		 std::lock_guard<std::mutex> lock(mutex_);
		 if (queue_.empty())
			 return false;

		 res = std::move(queue_.front());
		 queue_.pop_front();
		 return true;
	 }

	 bool TrySteal(Data& res) {
		 std::lock_guard<std::mutex> lock(mutex_);
		 if (queue_.empty())
			 return false;

		 res = std::move(queue_.back());
		 queue_.pop_back();
		 return true;
	 }
 private:
	 std::deque<Data> queue_;
	 mutable std::mutex mutex_;
};

}	// namespace internal

}	// namespace base
#endif // !BASE_WORK_STEALING_QUEUE_H
