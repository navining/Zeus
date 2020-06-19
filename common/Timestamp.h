#ifndef _Timestamp_h_
#define _Timestamp_h_

#include<chrono>
using namespace std::chrono;

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
	time_point<high_resolution_clock> _begin;
};

#endif // !_Timestamp_h_