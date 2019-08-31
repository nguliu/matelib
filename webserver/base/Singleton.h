// Author Fangping Liu
// 2019-08-29

#ifndef WEBSERVER_BASE_SINGLETON_H
#define WEBSERVER_BASE_SINGLETON_H

#include "noncopyable.h"
#include <pthread.h>
#include <stdlib.h> //atexit

namespace lfp
{
	//线程安全的单例类实现
	template<typename T>
	class Singleton : noncopyable
	{
	public:
		static T& instance()
		{
			pthread_once(&ponce_, &Singleton::init); //首次调用时执行init
			return *value_;
		}

	private:
		Singleton() {}
		~Singleton() {}

		static void init()
		{
			value_ = new T();
			::atexit(destroy); //登记一个销毁函数，在程序结束时自动调用
		}
		static void destroy()
		{
			delete value_;
		}

		static pthread_once_t ponce_; 	//这个变量能保证某个函数只被执行一次
		static T* value_; //单例对象
	};


template<typename T> pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;
template<typename T> T* Singleton<T>::value_ = nullptr;

} //namespace lfp

#endif //WEBSERVER_BASE_SINGLETON_H