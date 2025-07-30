#include "Inference.h"

void Inference::read(Ort::Session& session)
{
    auto inputShape = session.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    if (inputShape.size() != 4) return;
    char* inputName = session.GetInputNameAllocated(0, Ort::AllocatorWithDefaultOptions()).get();
    Input[0] = _strdup(inputName);
    N = inputShape[0], C = inputShape[1], H = inputShape[2], W = inputShape[3];
    char* outputName = session.GetOutputNameAllocated(0, Ort::AllocatorWithDefaultOptions()).get();
    Output[0] = _strdup(outputName);
}

void Inference::process(cv::Mat CVInputImage, cv::Mat* inputImage)
{
    cv::Mat f;
    cv::cvtColor(CVInputImage, f, cv::COLOR_RGBA2RGB);
    float w = f.cols, h = f.rows, s = float(min(W / w, W / h));
    int nw = int(w * s), nh = int(h * s);
    cv::Mat t;
    cv::resize(f, t, cv::Size(nw, nh), 0, 0, (f.cols > W) ? cv::INTER_AREA : cv::INTER_CUBIC);
    cv::copyMakeBorder(t, t, 0, W - nh, 0, W - nw, cv::BORDER_CONSTANT, cv::Scalar(101, 101, 101));
    vector<cv::Mat> v{ t };
    cv::Mat m;
    cv::merge(v, m);
    m.convertTo(*inputImage, CV_32F, 1.0 / 255.0, 0);
}

void Inference::detect(const cv::Mat& CVInputImage, const vector<uint8_t>& modelData, size_t* c, float** ret)
{
    if (ret == nullptr) return;
    if (CVInputImage.empty()) return;
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "Inference::detect");
    Ort::Session session(env, modelData.data(), modelData.size(), Ort::SessionOptions{ nullptr });
    read(session);
    Ort::MemoryInfo info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    const int size = H * W * C;
    size_t tensor_size = N * C * H * W;
    array<int64_t, 4> input_shape{ N, C, H, W };
    cv::Mat inputImage;
    process(CVInputImage, &inputImage);
    vector<float> temp(tensor_size);
    float* array = temp.data();
    float* source = (float*)(inputImage.data);
    for (int i = 0; i < size; i += C)
    {
        UINT32 index = i / C;
        for (int j = 0; j < C; ++j)
            array[(W * H * j) + index] = source[i + j];
    }
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(info, array, tensor_size, input_shape.data(), input_shape.size());
    vector<Ort::Value> output_list = session.Run(Ort::RunOptions{ nullptr }, Input, &input_tensor, 1, Output, 1);
    std::unique_ptr<Ort::Value> output_tensor = make_unique<Ort::Value>(move(output_list.front()));
    *c = output_tensor->GetTensorTypeAndShapeInfo().GetElementCount();
    float* out = output_tensor->GetTensorMutableData<float>();
    float* output = new float[*c]();
    for (size_t i = 0; i < *c; ++i)
        output[i] = out[i];
    *ret = output;
}