/*
 * fingering.h
 *
 *  Created on: 3 cze 2014
 *      Author: silmeth
 */

#ifndef FINGERING_H_
#define FINGERING_H_

#include "straightener.h"

Mat find_finger(int num, const Mat & trans_mat, const Mat & image,
		unsigned int rows, unsigned int cols);



#endif /* FINGERING_H_ */
