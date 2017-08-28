/*
 * opt_attr.h
 *
 *  Created on: Jan 15, 2016
 *      Author: eai
 */

#ifndef OPT_ATTR_H_
#define OPT_ATTR_H_

// This template is used to store optional attributes.
// Apart from the value, it has an attribute "is_set" to know whether it has been set or not.
template<typename T> struct opt_attr {
	T val;
	bool is_set;
	opt_attr(){
		this->is_set = false;
	};
	T operator=(T v) {
		this->is_set = true;
		val = v;
		return val;
	};
	operator T () {
		return val;
	};
};

#endif /* OPT_ATTR_H_ */
