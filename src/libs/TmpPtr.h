/*
 * TmpPtr.h
 *
 *  Created on: Sep 9, 2015
 *      Author: eai
 */

#ifndef TMPPTR_H_
#define TMPPTR_H_

template <class T>
class TmpPtr {
public:
	TmpPtr(T* ptr, int* counter) : ptr(ptr), counter(counter) {
		inc_cnt();	// Increment counter
	};
	TmpPtr(TmpPtr<T>& p) : ptr(p.ptr), counter(p.counter) {
		inc_cnt();	// Increment counter
	};
	TmpPtr(const TmpPtr<T>& p) : ptr(p.ptr), counter(p.counter) {
		inc_cnt();	// Increment counter
	};
	TmpPtr() : ptr(nullptr), counter(nullptr) {
	};
	virtual ~TmpPtr() {
		dec_cnt();	// Decrement counter
	};

	inline void inc_cnt(){
		if (counter) ++(*counter);
	}
	inline void dec_cnt(){
		if (counter) --(*counter);
	}

	TmpPtr<T>& operator =(const TmpPtr<T>& p) {
		dec_cnt();	// Decrement previous counter
		this->ptr = p.ptr;
		this->counter = p.counter;
		inc_cnt();	// Increment new counter
		return *this;
	};
	operator T*() {
		return ptr;
	};
	T* operator ->() {
		return ptr;
	};
private:
	T* ptr;
	int* counter;
};

#endif /* TMPPTR_H_ */
