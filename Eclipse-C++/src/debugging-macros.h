/*
 * debugging-macros.h
 *
 *  Created on: 3 Jul 2014
 *      Author: bas
 */

#ifndef DEBUGGING_MACROS_H_
#define DEBUGGING_MACROS_H_


#ifdef __OPTIMIZE__
#define __IF_DEBUG(x)
#define __IF_DEBUG__ if(0){
#define __ENDIF_DEBUG__ }
#undef __DEBUG__
#else
#define __IF_DEBUG(x) x
#define __IF_DEBUG__
#define __ENDIF_DEBUG__
#define __DEBUG__
#endif


#endif /* DEBUGGING_MACROS_H_ */
