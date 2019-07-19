#include "TV_Blending.h"

SingletonBlending* SingletonBlending::getInstance()
{
	static SingletonBlending singleton;
	return &singleton;
}

float calculateNewBlendedValue(float startValue,float endValue, float currentBlendTime, float fullBlendTime,bool rotation){
	if (currentBlendTime >= fullBlendTime){
		return endValue;
	}

	if (rotation){
		float diff = endValue - startValue;

		if (diff != 0.0){
			if (diff > 3.14159){
				diff -= 2*3.14159265358979323846;
			}
			if (diff < -3.14159){
				diff += 2*3.14159265358979323846;
			}
			float diffUnit = diff/fullBlendTime;
			float currentDiff = diffUnit * currentBlendTime;
			float BlendedValue = startValue + currentDiff;
			return BlendedValue;
		}else{
			return startValue;
		}
	}else{
		float diff = endValue - startValue;
		float diffUnit = diff/fullBlendTime;
		float currentDiff = diffUnit * currentBlendTime;
		float BlendedValue = startValue + currentDiff;
		return BlendedValue;
	}
}
