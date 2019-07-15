#ifndef _CLASSIFICATION_H_
#define _CLASSIFICATION_H_

#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace caffe; // NOLINT(build/namespaces)
using std::string;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

class Classifier
{
public:
  /***设置成单例模式，避免每次都需要重新加载模型**/
  static Classifier* getInstance(const string &model_file,
             const string &trained_file,
             const string &mean_file,
             const string &label_file)
	{
		if(m_pInstance == NULL)
		{
			m_pInstance = new Classifier(model_file,
           					    trained_file,
            					    mean_file,
             					    label_file);
			return m_pInstance;
		}
	}

  Classifier (const string &model_file,
             const string &trained_file,
             const string &mean_file,
             const string &label_file);

  std::vector<Prediction> Classify(const cv::Mat &img, int N = 5);

private:
	
  static Classifier* m_pInstance;

  void SetMean(const string &mean_file);

  std::vector<float> Predict(const cv::Mat &img);

  void WrapInputLayer(std::vector<cv::Mat> *input_channels);

  void Preprocess(const cv::Mat &img,
                  std::vector<cv::Mat> *input_channels);

  shared_ptr<Net<float>> net_;

  cv::Size input_geometry_;

  int num_channels_;

  cv::Mat mean_;
  
  std::vector<string> labels_;
};

#endif
