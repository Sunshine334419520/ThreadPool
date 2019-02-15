// UPUPOO
/**
* @Author: YangGuang
* @Date:   2018-09-29
* @Email:  guang334419520@126.com
* @Filename: thread_safe_queue.h
* @Last modified by:  YangGuang
*/

#ifndef __UPUPOO_THREAD_SAFE_QUEUE_H
#define __UPUPOO_THREAD_SAFE_QUEUE_H

#include <queue>
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "sun_exception.h"

namespace base {

// this is a thread safe queue.
template <class T>
class ThreadSafeQueue {
 public:
	 typedef T					value_type;
	 typedef value_type&		reference;
	 typedef value_type*		pointer;

 public:
	 ThreadSafeQueue() = default;
	 ~ThreadSafeQueue() = default;

	 ThreadSafeQueue(const ThreadSafeQueue& other) {
		 std::lock_guard<std::mutex> lk(other.mut);
		 contains_ = other.contains_;
	 }

	 // 禁止赋值运算符.
	 ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

	 // push.
	 void Push(T new_value) {
		 std::lock_guard<std::mutex> lk(mut);
		 contains_.push_back(std::move(new_value));
		 cond_data_.notify_one();
	 }

	 // 等待条件变量变为True，并且pop.
	 void WaitAndPop(T& value) {
		 std::unique_lock<std::mutex> lk(mut);
		 cond_data_.wait(lk, [this] { return !contains_.empty(); });

		 // 条件变量符合.
		 value = contains_.front();
		 contains_.pop_front();
	 }

	 // 智能指针代替.
	 std::shared_ptr<T> WaitAndPop() {
		 std::unique_lock<std::mutex> lk(mut);
		 cond_data_.wait(lk, [this] { return !contains_.empty(); });

		 // 条件变量符合
		 std::shared_ptr<T> res(std::make_shared<T>(contains_.front()));
		 contains_.pop_front();
		

		 return res;
	 }

	 // 尝试去pop，不管有无数据都会立即返回.
	 bool TryPop(T& value) {
		 std::lock_guard<std::mutex> lk(mut);
		 if (contains_.empty())
			 return false;

		 value = std::move(contains_.front());
		 contains_.pop_front();
		 
		 return true;
	 }

	 // 尝试去pop, 有数据放回，无数据放回nullptr shared_ptr.
	 std::shared_ptr<T> TryPop() {
		 std::lock_guard<std::mutex> lk(mut);
		 if (contains_.empty())
			 return std::shared_ptr<T>();

		 std::shared_ptr<T> res(std::make_shared<T>(contains_.front()));
		 contains_.pop_front();

		 return res;
	 }

	 bool Empty() const {
		 std::lock_guard<std::mutex> lk(mut);

		 return contains_.empty();
	 }

	 typename std::list<T>::iterator 
	 Erase(typename std::list<T>::iterator it) {
		 std::lock_guard<std::mutex> lk(mut);
		 return contains_.erase(it);
	 }

	 void Clear() {
		 std::lock_guard<std::mutex> lk(mut);
		 return contains_.swap(std::list<T>());
	 }

	 // 这是一个线程安全的删除所寻找的位置,
	 // return true 代表成功，否则失败.
	 template <class Predicate>
	 void FindAndErase(typename std::list<T>::iterator first,
					   typename std::list<T>::iterator last,
					   Predicate predicate) {
		 std::lock_guard<std::mutex> lk(mut);
		 typename std::list<T>::iterator it =
			 std::find_if(first, last, predicate);
		 if (it == contains_.end())
			 throw base::find_invalid();
		 contains_.erase(it);
	 }

	 // 这个寻找进行了范围的限制
	 template <class Predicate>
	 void FindAndErase(Predicate predicate) {
		 std::lock_guard<std::mutex> lk(mut);
		 typename std::list<T>::iterator it =
			 std::find_if(contains_.begin(), contains_.end(), predicate);
		 if (it == contains_.end())
			 throw base::find_invalid();
		 contains_.erase(it);
	 }

 private:
	 std::list<T> contains_;
	 std::condition_variable cond_data_;
	 // 设置为mutable 是为了在拷贝构造函数和Empty函数中可以使用.
	 mutable std::mutex mut;
};

}

#endif
