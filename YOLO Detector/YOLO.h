#pragma once
#include <string>

using namespace std;

struct YOLO_output
{
	float batch_id, x0, y0, x1, y1, cls_id, score;
	string msg = string("");
};