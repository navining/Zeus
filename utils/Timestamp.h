#ifndef _Timestamp_h_
#define _Timestamp_h_

#include<chrono>

class Time {
public:
	// Get current time in millisecond
	static time_t getCurrentTimeInMilliSec();
};

class Timestamp
{
public:
	Timestamp();

	~Timestamp();

	void update();

	// Get second
	double getElapsedSecond();

	// Get millisecond
	double getElapsedTimeInMilliSec();

	// Get microsecond
	long long getElapsedTimeInMicroSec();
protected:
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
};

#endif // !_Timestamp_h_