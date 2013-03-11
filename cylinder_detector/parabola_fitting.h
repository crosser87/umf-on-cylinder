#ifndef DP_PARABOLA_FITTING_H
#define DP_PARABOLA_FITTING_H

#include "geometry_fundamentals.h"

class CParabolaFitting
{
public:
  CParabolaFitting(TLine centralLine);
  ~CParabolaFitting(){}

  bool fitParabola(std::vector<cv::Point> points, TParabola& parabola, cv::Mat draw);
  void drawParabola(cv::Mat& img, TParabola parabola, cv::Scalar color, 
                    int thickness = 1);
  
  cv::Point2d transformPointBack(cv::Point input);
  
  void transformPointsToY(std::vector<cv::Point> input, 
                          std::vector<cv::Point2d>& output);
  
  void transformPointsBack(std::vector<cv::Point> input, 
                           std::vector<cv::Point>& output);
  
private:
  void getAngleAndOrigin(TLine line);
  void setupTrasfomationMatrices();
  
  cv::Mat transformationMatrix;
  cv::Mat transformationMatrixInverse;
  cv::Point2d origin;
  double angle;
};

#endif // DP_PARABOLA_FITTING_H
