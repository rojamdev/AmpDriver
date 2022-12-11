#pragma once

class FirstOrderIIR {
public:
	FirstOrderIIR() {
		lastSample = alpha = 0.0f;
		feedbackPolarity = 1.0f;
	}

	FirstOrderIIR(float alpha, bool isHighPass)
	{
		this->alpha = alpha;
		setType(isHighPass);
		lastSample = 0.0f;
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

	void setType(bool isHighPass)
	{
		feedbackPolarity = isHighPass ? -1.0f : 1.0f;
	}

	void reset()
	{
		lastSample = 0.0f;
	}

private:
	float alpha;
	float lastSample;
	float feedbackPolarity;
};