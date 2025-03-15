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
	virtual void on_parent_recalculate_transforms(); //called by owning object when its transforms are recalculated 
	virtual void on_parent_draw(); //called when parent object is drawn
	Object3D* get_parent() const;

protected:
	Object3D* parent;
};
