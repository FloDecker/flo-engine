#include "GlobalContext.h"


#include <iostream>

//TAG MANAGER 

TagManager::TagManager()
{
    for (auto c: ENGINE_TAGS)
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

////////////////////////////////////////////////////
