#ifndef _TV_Blending_H_
#define _TV_Blending_H_

#include "TV_NodeTreeView.h"
#include "STD_Types.h"

class SingletonBlending
{
private:
    SingletonBlending(){};

public:
	std::map<int,std::map<std::string,TV_NodeTreeViewPtr_t> > BlendingContainer;
	std::map<int,bool > blendingMissingObjectTracker;
	std::map<int,bool > blendingIdenticalNameTracker;

    static SingletonBlending* getInstance();
};

float calculateNewBlendedValue(float startValue,float endValue, float currentBlendTime, float fullBlendTime,bool rotation = false);

#endif
