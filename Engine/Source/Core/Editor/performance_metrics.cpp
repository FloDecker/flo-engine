#include "performance_metrics.h"

#include <ctime>
#include <GLFW/glfw3.h>

performance_metrics::performance_metrics()
{
	reset_measurements();
}

void performance_metrics::reset_measurements()
{
	measurements = std::vector<double>(_end_, 0.0);
	measurements_temp_storage = std::vector<double>(_end_, 0.0);
	histogram_ = std::vector<std::deque<double>>(_end_, std::deque<double>(histogram_recording_size_));
}

void performance_metrics::start_measuring(const metric metric)
{
	measurements_temp_storage[metric] = glfwGetTime();
}

double performance_metrics::stop_measuring_(const metric metric) const
{
	const auto m = measurements_temp_storage[metric];
	return static_cast<double>(glfwGetTime() - m);
}

double performance_metrics::stop_and_store_measuring(const metric metric)
{
	const auto result = stop_measuring_(metric);

	measurements[metric] = result;
	histogram_[metric].push_front(result);
	if (histogram_[metric].size() > histogram_recording_size_)
	{
		histogram_[metric].pop_back();
	}

	return result;
}

std::string performance_metrics::get_metric_as_string(metric metric) const
{
	return std::to_string(get_metric(metric));
}

std::string performance_metrics::get_average_metric_as_string(metric metric) const
{
	return std::to_string(get_average_metric(metric));
}

double performance_metrics::get_metric(metric metric) const
{
	return measurements[metric];
}

double performance_metrics::get_average_metric(metric metric) const
{
	if (histogram_[metric].empty())
	{
		return 0;
	}

	double sum = 0;
	for (double i : histogram_[metric])
	{
		sum += i;
	}
	return sum / static_cast<double>(histogram_[metric].size());
}
