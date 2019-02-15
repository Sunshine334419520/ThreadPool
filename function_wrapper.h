/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: function_wrapper.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_TASK_FUNCITONWRAPPER_H
#define BASE_TASK_FUNCITONWRAPPER_H

#include <utility>
#include <memory>

namespace base {

class FunctionWrapper {
public:
	FunctionWrapper() = default;

	template <typename F>
	FunctionWrapper(F&& f)
		: impl_(new ImplType<F>(std::move(f))) {}

	void operator()() { impl_->Run(); }

	FunctionWrapper(FunctionWrapper&& other)
		: impl_(std::move(other.impl_)) {}

	FunctionWrapper& operator=(FunctionWrapper&& other) {
		impl_ = std::move(other.impl_);
		return *this;
	}

	FunctionWrapper(const FunctionWrapper&) = delete;
	FunctionWrapper& operator=(const FunctionWrapper&) = delete;
private:
	
	struct ImplBase {
		virtual void Run() = 0;
		virtual ~ImplBase() {}
	};

	template <typename F>
	struct ImplType : ImplBase {
		F f;
		ImplType(F&& _f) : f(std::move(_f)) {}
		void Run() override { f(); }
	};

	std::unique_ptr<ImplBase> impl_;
};

}	// namespace base.
#endif // !BASE_TASK_FUNCITONWRAPPER_H