#pragma once

class FirstOrderIIR {
public:
	FirstOrderIIR() {
		lastSample = alpha = 0.0f;
		feedbackPolarity = 1.0f;
	}

	float processSample(float sample) 
	{
		float newSample = ((1 - alpha) * sample) + (feedbackPolarity * (alpha * lastSample));
		lastSample = newSample;
		return newSample;
	}

	void setAlpha(float alpha) 
	{
		this->alpha = alpha;
	}

	void setType(bool makeHighPass)
	{
		feedbackPolarity = makeHighPass ? -1.0f : 1.0f;
	}

private:
	float alpha;
	float lastSample;
	float feedbackPolarity;
};