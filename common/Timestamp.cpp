#include "Timestamp.h"

Timestamp::Timestamp()
{
	update();
}

Timestamp::~Timestamp() {}

void Timestamp::update()
{
	_begin = high_resolution_clock::now();
}

// Get second

double Timestamp::getElapsedSecond()
{
	return  getElapsedTimeInMicroSec() * 0.000001;
}

// Get millisecond

double Timestamp::getElapsedTimeInMilliSec()
{
	return this->getElapsedTimeInMicroSec() * 0.001;
}

// Get microsecond

long long Timestamp::getElapsedTimeInMicroSec()
{
	return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
}