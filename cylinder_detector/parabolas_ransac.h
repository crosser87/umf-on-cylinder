#ifndef DP_PARABOLA_RANSAC__H
#define DP_PARABOLA_RANSAC__H

#include <vector>

#include "ransac.h"
#include "geometry_fundamentals.h"

/**
 * Třída CRansacParabola
 *
 * Definuje RANSAC model pro paraboly definující hrany na válci.
 * 
 * Modelem je vzdálenost od přímky, která se fituje na body určené parametrem 
 * zakřivení a Y-ovou souřadnicí středu.
 * 
 * Inherits from generic CRansac.
 *
 * @author Radim Kriz (xkrizr03@stud.fit.vutbr.cz)
 */
class CRansacParabola : public CRansac<TParabola>
{
public:

  /**
   * Konstruktor CRansacParabola
   *
   * @param     int _numberOfIteration                  počet iterací
   * @param     int _modelDistanceTrashold              vzdálenostní práh
   */
  CRansacParabola(int _numberOfIteration,
                  double _modelDistanceTrashold = 5
  );
  
  virtual ~CRansacParabola(){}      // virtual destructor
  
  /**
   * Metoda fitParabolaRANSAC
   *
   * Provede kompletní algoritmus RANSAC. Po algoritmu provede spřesnění 
   * hypotézy ze všech inliers.
   *
   * @param     std::vector<TParabola> ellipses          vstupní paraboly
   * @param     std::vector<TParabola>& inliers          výstupní inliers
   * 
   * @result    int                                    výsledné skóre
   */
  int fitParabolaRANSAC(std::vector<TParabola> parabolas,
                       std::vector<TParabola>& inliers);
  
protected:

  /**
   * Metoda fitRansacModel
   *
   * Testuje jestli vstupní parabola vyhovuje hypotéze
   *
   * @param     TParabola modelParabola      modelová parabola
   *
   * @result    bool                       pravda pokud parabola vyhovuje
   */
  virtual bool fitRansacModel(TParabola modelParabola);
  
  /**
   * Metoda isModel
   *
   * Definuje model pro RANSAC iteraci. Stačí dvě paraboly.
   *
   * @param     std::vector<TParabola> modelParabolas    data k vytvoření modelu
   *
   * @result    bool                                     pravda pokud je vše OK
   */
  virtual bool isModel(std::vector<TParabola> modelParabolas);
  
private:
  
  /**
   * Metoda recomputeParams
   *
   * Přepočítá hypotézu ze všech inliers
   *
   * @param     std::vector<TParabola> inliers    inliers
   */
  void recomputeParams(std::vector<TParabola> inliers);
  
  /**
   * Metoda getFinalInliers
   *
   * Hledá finální inliers ze spřesnných parametrů
   *
   * @param     std::vector<TParabola> ellipses          paraboly
   * @param     std::vector<TParabola>& inliers          inliers
   */  
  void  getFinalInliers(std::vector<TParabola> parabolas,
                        std::vector<TParabola>& inliers);
  
public:
  double modelDistanceTrashold; // vzdálenostní práh
  TLine testLine;               // testovací přímka
};

#endif // DP_PARABOLA_RANSAC__H

