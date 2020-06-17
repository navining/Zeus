#ifndef _Timestamp_hpp_
#define _Timestamp_hpp_

#include<chrono>
using namespace std::chrono;

class Timestamp
{
public:
	Timestamp()
	{
		update();
	}

	~Timestamp() {}

	void update()
	{
		_begin = high_resolution_clock::now();
	}

	// Get second
	double getElapsedSecond()
	{
		return  getElapsedTimeInMicroSec() * 0.000001;
	}

	// Get millisecond
	double getElapsedTimeInMilliSec()
	{
		return this->getElapsedTimeInMicroSec() * 0.001;
	}

	// Get microsecond
	long long getElapsedTimeInMicroSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}
protected:
	time_point<high_resolution_clock> _begin;
};

#endif // !_Timestamp_hpp_