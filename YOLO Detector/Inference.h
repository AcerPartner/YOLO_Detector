#pragma once
#include <onnxruntime_cxx_api.h>
#include <cpu_provider_factory.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#define NOMINMAX
#include "framework.h"
#include "YOLO.h"

class Inference
{
public:
	void detect(const cv::Mat& image, const vector<uint8_t>& modelData, vector<YOLO_output>* ret);
	void read(Ort::Session& session);
	void process(cv::Mat original, cv::Mat* ret);
	int N;
	int C;
	int H;
	int W;
	const char* Input[1];
	const char* Output[1];
};