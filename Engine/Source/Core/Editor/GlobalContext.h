#pragma once
#include <string>
#include <vector>
#include "../Renderer/Shader/ShaderProgram.h"

class TagManager
{
public:
    TagManager();
    int add_tag(std::string);    
    int get_id_of_tag(const std::string& tag) const;
    
private:
    std::vector<std::string> tags_;
    const std::vector<const char*> ENGINE_TAGS{"ENGINE_LIGHT_POINT", "ENGINE_COLLIDER"};
};

//Global context holds information that is needed across levels, such as tags 
struct GlobalContext
{
    TagManager tag_manager;
    ShaderProgram *default_shader;
};
