#pragma once

class Object3D;

//a modifier is attached to an 3D object but doesn't have its own transform  
class modifier
{
public:
	virtual ~modifier() = default;
	modifier(Object3D* parent_game_object_3d);
	virtual void draw_gui();
	virtual int get_id();
	Object3D *get_parent() const;
	
protected:
	Object3D* parent;
	
	
};
