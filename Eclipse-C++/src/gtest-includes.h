/*
 * gtest-includes.h
 *
 *  Created on: 29 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef GTEST_INCLUDES_H_
#define GTEST_INCLUDES_H_

#ifndef __OPTIMIZE__
// Code compiled with debugging flags - include Google Test code
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#define ASSERT_DOUBLE_NEAR(val1,val2) ASSERT_NEAR(val1,val2,0.000001)
#endif /* __OPTIMIZE__ */


#endif /* GTEST_INCLUDES_H_ */
