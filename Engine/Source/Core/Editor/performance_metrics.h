#pragma once
#include <ctime>
#include <deque>
#include <map>
#include <string>
#include <vector>

enum metric
{
	tick_cycle_time, //time in ms for one tick
	pass_main,
	pass_g_buffer,
	pass_surfels,
	pass_direct_light,
	physics,
	surfels_tick,

	_end_, //for iterating
};

class performance_metrics
{
public:
	performance_metrics();
	std::vector<double> measurements;
	std::vector<double> measurements_temp_storage;

	void reset_measurements();
	void start_measuring(metric metric);
	double stop_and_store_measuring(metric metric);
	std::string get_metric_as_string(metric metric) const;
	std::string get_average_metric_as_string(metric metric) const;
	double get_metric(metric metric) const;
	double get_average_metric(metric metric) const;

private:
	double stop_measuring_(metric metric) const;

	unsigned int histogram_recording_size_ = 30;
	std::vector<std::deque<double>> histogram_;

};
