#pragma once

class User
{
public:
	int getPos()
	{
		return pos; 
	}
	void setPos(int newPos)
	{
		pos = newPos;
	}
private:
	int pos; 
};