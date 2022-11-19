#pragma once

enum class eCurveStyle
{
	Linear,
	Square,
};

template<class T>
class AnimationCurve
{
public:
	// Takes time value and returns animation curve value at position.
	T Value(float aTime);
	void Add(const T& aValue, float aTime);

	void SetStyle(eCurveStyle aCurveStyle);
	
	int Keys() const;
private:
	struct AnimationKey
	{
		T value;
		float time;
	};

	eCurveStyle myStyle = eCurveStyle::Linear;
	std::vector<AnimationKey> myKeys;
};

template <class T>
void AnimationCurve<T>::Add(const T& aValue, float aTime)
{
	const AnimationKey key{ aValue, aTime};

	for (int i = 0; i < myKeys.size(); i++)
	{
		if (myKeys[i].time > aTime)
		{
			myKeys.insert(myKeys.begin() + i, key);
			return;
		}
	}

	myKeys.emplace_back(key);
}

template <class T>
void AnimationCurve<T>::SetStyle(eCurveStyle aCurveStyle)
{
	myStyle = aCurveStyle;
}

template <class T>
int AnimationCurve<T>::Keys() const
{
	return myKeys.size();
}

template <class T>
T AnimationCurve<T>::Value(float aTime)
{
	if (myKeys.size() == 0)
	{
		return T();
	}

	if (aTime < myKeys[0].time)
	{
		return myKeys[0].value;
	}
	
	for (int i = 1; i < myKeys.size() && myKeys.size() > 1; i++)
	{
		if (myKeys[i].time > aTime)
		{
			if (myStyle == eCurveStyle::Square)
			{
				return myKeys[i - 1].value;
			}
			
			T valStart = myKeys[i - 1].value;
			T valEnd = myKeys[i].value;

			const float startTime = myKeys[i - 1].time;
			const float endTime = myKeys[i].time;

			const float timeDiff = endTime - startTime;
			const float time = (aTime - startTime) / timeDiff;

			return (valStart + ((valEnd - valStart) * time));
		}
	}

	return myKeys[myKeys.size() - 1].value;
}
