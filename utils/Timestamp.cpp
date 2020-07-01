#include "Timestamp.h"

using namespace std::chrono;

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

time_t Time::getCurrentTimeInMilliSec()
{
	return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}
