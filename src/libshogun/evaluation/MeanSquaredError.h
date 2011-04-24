/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2011 Sergey Lisitsyn
 * Copyright (C) 2011 Berlin Institute of Technology and Max-Planck-Society
 */

#ifndef MEANSQUAREDERROR_H_
#define MEANSQUAREDERROR_H_

#include "evaluation/Evaluation.h"
#include "features/Labels.h"

namespace shogun
{

/** @brief the class MeanSquaredError
 * used to compute error of regression model.
 *
 * Formally, for labels \f$ L,R, |L|=|R|\f$ mean squared error is estimated as
 *
 * \f[
 * 		\frac{1}{|L|} \sum_{i=1}^{|L|} (L_i - R_i)^2
 * \f]
 *
 */
class CMeanSquaredError: public CEvaluation
{
public:
	/** constructor */
	CMeanSquaredError() : CEvaluation() {};

	/** destructor */
	virtual ~CMeanSquaredError() {};

	/** evaluate mean squared error
	 * @param predicted labels for evaluating
	 * @param ground_truth labels assumed to be correct
	 * @return mean squared error
	 */
	virtual float64_t evaluate(CLabels* predicted, CLabels* ground_truth);

	/** get name */
	virtual inline const char* get_name() const { return "MeanSquaredError"; }
};

}

#endif /* MEANSQUAREDERROR_H_ */