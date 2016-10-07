#ifndef _DRBUG_H_
#define	_DRBUG_H_

#define Debug(__str__)
#define Error(__str__) std::cout<<__str__<<std::endl
#define NILPTR NULL

namespace DrBug
{


	// usefull types
	// typedef enum {false,true}bool;
	typedef enum {STERR,STFAIL,STOK}Status;

} // namespace DrBug

#endif // _DRBUG_H_
