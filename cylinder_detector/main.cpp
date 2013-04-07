#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>

#include "edges.h"
#include "edgels.h"
#include "lines_fitting.h"
#include "ellipses_fitting.h"
#include "ellipses_ransac.h"
#include "umf_wrapper.h"
#include "lines_clustering.h"
#include "ellipses_clustering.h"
#include "parabolas_fitting.h"
#include "parabolas_ransac.h"
#include "parabolas_clustering.h"
#include "supplement.h"
#include "find_grid.h"

using namespace cv;
using namespace std;

int scanlineStep = 50;
int searchStep = 10;
int searchRadius = 5;

int main(int argc, char** argv)
{
  CFindEdges* findEdges = new CFindEdges(scanlineStep, 15, 25);
  CFindEdgels* findEdgels = new CFindEdgels(searchStep, searchRadius, 25);
  CFittingLine* lineFitting = new CFittingLine();
  CEllipseFitting* ellipseFitting = new CEllipseFitting();
  CWrapper* wrapper = new CWrapper();

  // output file
  ofstream myfile;
  myfile.open ("output.txt");
  //int counter = 0;
  
  for(int x = 1; x < 1000; x++) {

    stringstream str;
    str << "../data/images5/" << x << "data.jpg";
    //str << "data/test.jpg";
    cout << str.str() << endl;

    Mat source = imread(str.str());
    Mat draw = imread(str.str());

    if(source.data == NULL)
    {
      cout << "--------- NO MORE IMAGES -------" << endl;
      break;
    }
    
    if( source.empty()) break;

    Mat gray;
    cvtColor(source, gray, CV_BGR2GRAY);
    
    // detekce hran na mrizce
    vector<Point2f> edges;    
    findEdges->findEdges(gray, edges);
    
    // detekce edgelu
    vector<vector<Point2f> > newEdges;   
    findEdgels->getEdgesFromEdgePoints(gray, edges, newEdges, draw);
    
    // fittovani primek
    vector<TLine> lines;
    lineFitting->fitLines(newEdges, lines);
    
    // 2 main lines direction location
    vector<TLine> linesGrouped;
    vector<TLine> linesGrouped2;
    wrapper->setCenter(Point2f(source.cols / 2, source.rows / 2));
    int index = wrapper->getLineGroups(lines, linesGrouped, linesGrouped2);
    
    // fitting ellipses
    vector<TEllipse> ellipses;
    ellipseFitting->fitEllipsesFromLines(linesGrouped2, ellipses);
 
    // location of vanishing point and lines correction
    vector<TLine> linesSelected;
    TLine vanishNormal;
    Point2f vanishPoint = wrapper->GetVanishingPoint(linesGrouped, index, linesSelected, vanishNormal, Point2f(source.cols / 2, source.rows / 2));

    cout << "vanishPoint: " << vanishPoint << endl;
    
    TLine centralLine = TLine(Vec4f(vanishNormal.a, vanishNormal.b, vanishPoint.x, vanishPoint.y));
    
////////////////////////////////////////////////////////////////////////////////
    Mat rgb;
    source.copyTo(rgb);

    for(int i = 0; i < (int)lines.size(); i++)
    {
      drawLine(rgb, lines[i], Scalar(255, 255, 0));
    }

////////////////////////////////////////////////////////////////////////////////

    Mat rgb2;
    source.copyTo(rgb2);
    
    // points, all ellipses    
    for(int i = 0; i < (int)linesGrouped.size(); i++)
    {
      drawLine(rgb2, linesGrouped[i], Scalar(255, 255, 0));
    }
    
    // points, all ellipses    
    for(int i = 0; i < (int)linesGrouped2.size(); i++)
    {
      drawLine(rgb2, linesGrouped2[i], Scalar(0, 255, 255));
    }

////////////////////////////////////////////////////////////////////////////////

    Mat rgb3;
    source.copyTo(rgb3);
    
    drawLine(rgb3, centralLine, Scalar(255, 0, 255), 2);
    
    for(int i = 0; i < (int)linesSelected.size(); i++)
    {
      drawLine(rgb3, linesSelected.at(i), Scalar(255, 255, 0));
    }     

////////////////////////////////////////////////////////////////////////////////

    Mat rgb5;
    source.copyTo(rgb5);

    for (int i=0;i != (int)ellipses.size(); ++i) 
    {
      ellipse(rgb5, ellipses.at(i).boundingBox, Scalar(255, 255, 0));
      drawPoint(rgb5, ellipses.at(i).mainEdge, Scalar(0, 0, 255));
    }
    
////////////////////////////////////////////////////////////////////////////////    
    Mat rgb7;
    source.copyTo(rgb7);   
    
    // points, all ellipses
    int inliersNumber;
    CRansacEllipse* ransac = new CRansacEllipse(100, vanishPoint, 10, 10, 3.0);
    vector<TEllipse> inliers;  
    TLine cylinderCentralLine, pyramideLine;
    inliersNumber = ransac->fitEllipseRANSAC(ellipses, inliers, cylinderCentralLine, pyramideLine);
    
    cout << "Number of inliers: " << inliersNumber << endl;
    
    drawLine(rgb7, cylinderCentralLine, Scalar(0, 255, 255));
    drawLine(rgb7, pyramideLine, Scalar(0, 255, 255));
    drawLine(rgb7, ransac->elipseMainAxeLine, Scalar(0, 255, 255));
    
    for (int i=0;i != (int)inliers.size(); ++i)
    {  
      ellipse(rgb7, inliers.at(i).boundingBox, Scalar(255, 255, 0), 1);  
      drawPoint(rgb7, inliers.at(i).mainEdge, Scalar(0, 0, 255));
      drawPoint(rgb7, inliers.at(i).secondaryEdge, Scalar(0, 0, 255));
      /*
      cout << "center: " << inliers.at(i).boundingBox.center << endl;
      cout << "size: " << inliers.at(i).boundingBox.size << endl;
      cout << "angle: " << inliers.at(i).boundingBox.angle << endl;
      */
    }
      
    //////////////////////////////////////////////////////////////////////////////// 

    Mat rgb8;
    source.copyTo(rgb8);   
    
    vector<TEllipse> finalEllipses;
    CEllipseClustring* clusteringellipse = new CEllipseClustring();
    clusteringellipse->runEllipsesClustering(inliers, finalEllipses);
    
    for(int i = 0; i < (int)finalEllipses.size(); i++)
    {
      ellipse(rgb8, finalEllipses.at(i).boundingBox, Scalar(255, 255, 0));
      
      /*
      cout << "final center: " << finalEllipses.at(i).boundingBox.center << endl;
      cout << "final size: " << finalEllipses.at(i).boundingBox.size << endl;
      cout << "final angle: " << finalEllipses.at(i).boundingBox.angle << endl;
      */
      
    }    
    //////////////////////////////////////////////////////////////////////////////// 
    
    Mat rgb4;
    source.copyTo(rgb4);
    
    
    vector<TLine> finallines;
    CLineClustring* clustering = new CLineClustring(cylinderCentralLine, pyramideLine);
    cout << "number of selectedLines: " << linesSelected.size() << endl; 
    clustering->runLinesClustering(linesSelected, finallines);
    
    for(int i = 0; i < (int)finallines.size(); i++)
    {
      drawLine(rgb4, finallines[i], Scalar(255, 255, 0));
    }     
    
    ////////////////////////////////////////////////////////////////////////////////   
    
    Mat rgb9;
    source.copyTo(rgb9);
    
    CParabolaFitting* parabolaFitting = new CParabolaFitting(cylinderCentralLine);
    
    drawLine(rgb9, centralLine, Scalar(255, 0, 255));
    
    vector<TParabola> parabolas;
    parabolaFitting->fitParabolas(linesGrouped2, parabolas);
    
    for(int i = 0; i < (int)parabolas.size(); i++)
    {
      parabolaFitting->drawParabola(rgb9, parabolas.at(i), Scalar(255, 255, 0));
    }  
    
    ////////////////////////////////////////////////////////////////////////////////   
    
    Mat rgb10;
    source.copyTo(rgb10);
        
    CRansacParabola* parabolaRansac = new CRansacParabola(100, 5e-4);
    vector<TParabola> inliersParabola;
    inliersNumber = parabolaRansac->fitParabolaRANSAC(parabolas, inliersParabola);
    
    cout << "Number of inliers: " << inliersNumber << endl;
    
    for(int i = 0; i < (int)inliersParabola.size(); i++)
    {
      parabolaFitting->drawParabola(rgb10, inliersParabola.at(i), Scalar(255, 255, 0));
    }
    
    ////////////////////////////////////////////////////////////////////////////////   

    Mat rgb11;
    source.copyTo(rgb11);
    
    CParabolaClustring* parabolaClustring = new CParabolaClustring();
    
    vector<TParabola> clusteredParabola;
    parabolaClustring->runParabolasClustering(inliersParabola, clusteredParabola);
    
    vector<TParabola> clusteredFinalParabola;
    Point2f referencePoint = parabolaClustring->recomputeClusteredParabolas(clusteredParabola, clusteredFinalParabola);
    
    for(int i = 0; i < (int)clusteredFinalParabola.size(); i++)
    {
      parabolaFitting->drawParabola(rgb11, clusteredFinalParabola.at(i), Scalar(255, 255, 0));
    }
    
    drawPoint(rgb11, parabolaFitting->transformPointBack(referencePoint), Scalar(0, 0, 255));
    
    /*
    //myfile << "X" << counter++ << " = [" << endl;
    //int counter2 = 0;
    
    
    sort(clusteredParabola.begin(), clusteredParabola.end(), [](TParabola a, TParabola b){return a.apex.y > b.apex.y;});
    
    for(int i = 2; i < (int)clusteredParabola.size()-2; i++)
    {
      parabolaFitting->drawParabola(rgb11, clusteredParabola.at(i), Scalar(255, 255, 0));
      
      //cout << "pos: " << clusteredParabola.at(i).apex.y << "," << "pos: " << (clusteredParabola.at(i).apex.y - clusteredParabola.at(i+1).apex.y) << endl;
      cout << "param: " << clusteredParabola.at(i).param << "," << "param diff: " << (clusteredParabola.at(i).param - clusteredParabola.at(i+1).param) << endl;
      
      //myfile << (clusteredParabola.at(i).apex.y - tmpValue) << endl;
      //myfile << "--------------------------" << endl;
      //tmpValue = clusteredParabola.at(i).apex.y;
    }
    
    double refDist = clusteredParabola.at(3).apex.y - clusteredParabola.at(2).apex.y;
    
    double sum = 0;
    int counter2 = 0;
    for(int i = 4; i < (int)clusteredParabola.size()-2; i++)
    {
      double tmpDist = clusteredParabola.at(i).apex.y - clusteredParabola.at(i-1).apex.y;
      sum += (refDist - tmpDist) / (i - 3);
      counter2++;
      cout << "(refDist - tmpDist) / (i - 3): " << (refDist - tmpDist) / (i - 3) << endl;
    }
    
    double tmpSize = sum / counter2;
    double tmpPos = clusteredParabola.at(2).apex.y;
    double tmpPos2 = clusteredParabola.at(2).apex.y;
    
    cout << "tmpSize: " << tmpSize << endl;
    cout << "tmpdist: " << refDist << endl;
        
    parabolaFitting->drawParabola(rgb11, TParabola(Point2f(0, tmpPos2), 0, clusteredParabola.at(2).angle, clusteredParabola.at(2).origin), Scalar(255, 0, 255));
    
    int counter3 = 0;  
    for(int i = 0; i < 20; i++)
    {
      cout << "refDist + counter3*tmpSize: " << refDist + counter3*tmpSize << endl;

      tmpPos2 -= refDist + (counter3 * tmpSize);
      parabolaFitting->drawParabola(rgb11, TParabola(Point2f(0, tmpPos2), 0, clusteredParabola.at(2).angle, clusteredParabola.at(2).origin), Scalar(255, 0, 255));
      
      tmpPos += refDist - (counter3 * tmpSize);
      parabolaFitting->drawParabola(rgb11, TParabola(Point2f(0, tmpPos), 0, clusteredParabola.at(2).angle, clusteredParabola.at(2).origin), Scalar(255, 0, 255));
      counter3++;
    }
    */
    //myfile << "]" << endl << endl;
    ////////////////////////////////////////////////////////////////////////////////     
 
 
 Mat rgb12;
 source.copyTo(rgb12);
 
 CSupplement* supplement = new CSupplement();
 vector<TParabola> supplementParabola;
 supplement->runSupplement(clusteredFinalParabola, supplementParabola, referencePoint);
 
 for(int i = 0; i < (int)supplementParabola.size(); i++)
 {
   parabolaFitting->drawParabola(rgb12, supplementParabola.at(i), Scalar(255, 255, 0));
 }
 
 /*
  * vector<TParabola> clusteredParabola2;
  for(int i = 0; i < (int)clusteredParabola.size(); i++)
  {
    clusteredParabola2.push_back(clusteredParabola.at(i));
  }
  */
 /*
  f o*r(int i = 0; i < (int)clusteredParabola2.size(); i++)
  {
    parabolaFitting->drawParabola(rgb12, clusteredParabola2.at(i), Scalar(255, 0, 255));
  }
  */
 
 ////////////////////////////////////////////////////////////////////////////////     
 
 Mat rgb13;
 source.copyTo(rgb13);
 
 CFindGrid* findGrid = new CFindGrid(parabolaFitting->transformationMatrix, parabolaFitting->transformationMatrixInverse, referencePoint);
 vector<vector<Point2f> > gridPopints;
 
 findGrid->findGrid(finallines, supplementParabola, gridPopints);
 
 for(int i = 0; i < (int)supplementParabola.size(); i++)
 {
   parabolaFitting->drawParabola(rgb13, supplementParabola.at(i), Scalar(255, 255, 0));
 }
 
 for(int i = 0; i < (int)finallines.size(); i++)
 {
   drawLine(rgb13, finallines.at(i), Scalar(255, 255, 0));
 } 
 
 for(int i = 0; i < (int)gridPopints.size(); i++)
 {
   for(int j = 0; j < (int)gridPopints.at(i).size(); j++)
   {
     drawPoint(rgb13, gridPopints.at(i).at(j), Scalar(0, 0, 255));
   }
 } 
 
 ////////////////////////////////////////////////////////////////////////////////  
 
    delete parabolaFitting;
    delete ransac;
    delete clustering;
    delete parabolaRansac;
    delete parabolaClustring;
    
    cout << "-------" << x << "-------" << endl;

    imshow("Output: All lines", rgb);
    imshow("Output: Lines grouped by direction", rgb2);
    imshow("Output: Lines after fitting vanishing point", rgb3);
    imshow("Output: lines after clustering", rgb4);

    imshow("Output: All ellipses", rgb5);
    imshow("Output: Ellipses after RANSAC", rgb7);
    imshow("Output: Ellipses after clustering", rgb8);
    
    imshow("Output: Parabolas", rgb9);
    imshow("Output: Parabolas ransac", rgb10);
    imshow("Output 11: Parabolas clustering", rgb11);
    
    imshow("Output 12: Parabolas supplement", rgb12);
    
    stringstream str1;
    str1 << "all-lines" << x << ".png";
    stringstream str2;
    str2 << "grouped-lines" << x << ".png";
    stringstream str3;
    str3 << "vanishing-lines" << x << ".png";
    stringstream str4;
    str4 << "clustered-lines" << x << ".png";

    stringstream str5;
    str5 << "all-ellipses" << x << ".png";
    stringstream str7;
    str7 << "ransac-ellipses" << x << ".png";
    stringstream str8;
    str8 << "clustering-ellipses" << x << ".png";

    stringstream str9;
    str9 << "parabolas-all" << x << ".png";
    stringstream str10;
    str10 << "parabolas-ransac" << x << ".png";
    stringstream str11;
    str11 << "parabolas-clustering" << x << ".png";
    
    imwrite(str1.str(), rgb);
    imwrite(str2.str(), rgb2);
    imwrite(str3.str(), rgb3);
    imwrite(str4.str(), rgb4);

    imwrite(str5.str(), rgb5);
    imwrite(str7.str(), rgb7);
    imwrite(str8.str(), rgb8);
    
    imwrite(str9.str(), rgb9);
    imwrite(str10.str(), rgb10);
    imwrite(str11.str(), rgb11);
    
    uchar c = (uchar)waitKey();

    if(c == 27)
    {
      break;
    }
  }

  myfile.close();
  
  delete findEdges;
  delete findEdgels;
  delete lineFitting;
  delete ellipseFitting;
  
  return 0;
}
