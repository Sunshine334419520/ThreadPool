// UPUPOO
/**
* @Author: YangGuang
* @Date:   2018-10-08
* @Email:  guang334419520@126.com
* @Filename: sun_exception.h
* @Last modified by:  YangGuang
*/
#ifndef __UPUPOO_SUN_EXCEPTION_H
#define __UPUPOO_SUN_EXCEPTION_H

#include <exception>
#include <iostream>

namespace base {

struct find_invalid : std::exception {
	const char* what() const {
		return "Find invalid!";
	}
};

struct empty_stack : std::exception {
	const char* what() const throw() {
		return  "This is empty stack!";
	}
};


}					// namespace base


#endif