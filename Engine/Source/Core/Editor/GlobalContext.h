#pragma once
#include <string>
#include <vector>


class TagManager
{
public:
    TagManager();
    int add_tag(std::string);    
    int get_id_of_tag(std::string tag) const;
    
private:
    std::vector<std::string> tags_;
    const std::vector<const char*> ENGINE_TAGS{"ENGINE_LIGHT_POINT"};
};


struct GlobalContext
{
    TagManager tag_manager;
};
