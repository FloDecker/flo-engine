#pragma once

enum primitive_type
{
    undefined,
    mesh,
    sphere,
    plane,
    box,
    line
};

class primitive {
private:
    bool isLoaded = false;
    bool mesh_calculation_ran = false;
    void run_mesh_calculations();


public:
    virtual primitive_type get_primitive_type() { return undefined; }
    virtual int load() = 0;
    virtual int draw() = 0;

    template <typename T>
    T* as()
    {
        return (get_primitive_type() == T::Type) ? dynamic_cast<T*>(this) : nullptr;
    }
};



