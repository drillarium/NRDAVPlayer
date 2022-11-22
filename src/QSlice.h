#ifndef __QSLICE_H
#define __QSLICE_H

#include <QWidget>
#include <QSplitter>

class QSlice : public QWidget
{
  Q_OBJECT

private:
  QWidget		*_before;		// espace avant
  QWidget		*_selection;	// espace selectionné
  QWidget		*_after;		// espace après
  QSplitter	*_splitter;		// barres de sélection

  double		_currentMin;	// valeur minimale courante
  double		_currentMax;	// valeur maximale courante
  double		_valueMin;		// valeur minimale limite
  double		_valueMax;		// valeur maximale limite

public:
  explicit QSlice(QWidget *parent = nullptr);	// constructeur
  ~QSlice();						// destructeur

  QSize sizeHint() const;			// taille par defaut

  //getters
  int currentMin();
  int currentMax();
  int valueMin();
  int valueMax();

  //setters
  void setCurrentMin(int currentMin);
  void setCurrentMax(int currentMax);
  void setValueMin(int valueMin);
  void setValueMax(int valueMax);
  void setBeforeColor(const QColor &color);	// couleur avant
  void setSelectionColor(const QColor &color);// couleur selection
  void setAfterColor(const QColor &color);	// couleur après

  void setMinMax(int vMin, int vMax, int curMin, int curMax)
  { 
    _valueMin = vMin;
    _valueMax = vMax;
    _currentMin = curMin;
    _currentMax = curMax;
    updateMinPos();
    updateMaxPos();
  }  

protected:
  void updateMinPos();	// recale le curseur du minimum
  void updateMaxPos();	// recale le curseur du maximum

protected slots:
  void updateValues(int pos, int index);	// mise a jour des valeurs
  void printVals(int = 0);

signals:
  void minChanged(int);	// minimum changé
  void maxChanged(int);	// maximum changé
};

#endif
