#pragma once

class texture
{
public:
	void use(unsigned int textureUnit) const;

protected:
	unsigned int texture_;
	bool initialized_ = false;
};
