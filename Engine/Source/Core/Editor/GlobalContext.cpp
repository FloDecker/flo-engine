#include "GlobalContext.h"


#include <iostream>

//TAG MANAGER 

TagManager::TagManager()
{
	for (auto c : ENGINE_TAGS)
	{
		std::string str(c);
		tags_.push_back(str);
	}
	std::cout << "added default tags" << '\n';
}

int TagManager::add_tag(std::string tag)
{
	if (get_id_of_tag(tag) >= 0)
	{
		std::wcout << "tag " << tag.c_str() << " already exists" << '\n';
		return -1;
	}
	tags_.push_back(tag);
	return 0;
}

//returns -1 if tag doesent exist
int TagManager::get_id_of_tag(const std::string& tag) const
{
	for (unsigned int i = 0; i < tags_.size(); i++)
	{
		if (tags_.at(i) == tag)
		{
			return i;
		}
	}

	return -1;
}

unsigned int Logger::get_current_log_amount() const
{
	return log_array_.size();
}

std::vector<log_entry>* Logger::get_log_entries()
{
	return &log_array_;
}

void Logger::print_to_log(const log_type type, std::string log)
{
	if (log_array_.size() > max_logs)
	{
	}
	log_array_.emplace_back(log_entry(type, log));
}

void Logger::print_info(std::string log)
{
	print_to_log(log_info, log);
}
