#ifndef __ALGORITHM_REGISTRATION__H_
#define __ALGORITHM_REGISTRATION__H_

#include <functional>

#include "Direction.h"
#include "AbstractAlgorithm.h"
#include "MakeUnique.h"

class AlgorithmRegistration
{
public:
	AlgorithmRegistration(std::function<unique_ptr<AbstractAlgorithm>()>);
};

#define REGISTER_ALGORITHM(class_name) \
AlgorithmRegistration register_me_##class_name( \
	[]{return make_unique<class_name>();} \
);


#endif //__ALGORITHM_REGISTRATION__H_