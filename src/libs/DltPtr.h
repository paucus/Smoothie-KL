/*
 * DltPtr.h
 *
 *  Created on: Sep 3, 2015
 *      Author: eai
 */

#ifndef DLTPTR_H_
#define DLTPTR_H_

/**
 * This class holds a pointer to an object that is initialized as null. When it's accessed for the
 * first time, a new instance of the object class is created and it's pointer is kept.
 * Calling flush_obj deletes the referenced object.
 */
template<class T>
class DltPtr {
public:
	DltPtr() : p(nullptr) {};
	virtual ~DltPtr() {};

	DltPtr<T>& operator=(T*p) {
		this->p = p;
		return *this;
	};
	operator T*() {
		return instance();
	};
	T& operator *() {
		return *instance();
	};
	operator bool() {
		return p != nullptr;
	};
	T* operator ->() {
		return instance();
	};
	T* instance() {
		if (!p)
			p = new T();
		return p;
	};
	bool is_ptr(void* p_arg) {
		return p == p_arg;
	};
	void flush_obj(){
		if (p){
			delete p;
			p = nullptr;
		}
	};
private:
	T * p;
};

#endif /* DLTPTR_H_ */
