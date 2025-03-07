#pragma once


class primitive {
private:
    bool isLoaded = false;
    bool mesh_calculation_ran = false;
    void run_mesh_calculations();


public:
    virtual int load() = 0;
    virtual int draw() = 0;
};



