#include "qSlice.h"

#include <QGridLayout>
#include <QDebug>

// contructeur
QSlice::QSlice(QWidget *parent)
  : QWidget(parent)
{
  // instanciations
  _before = new QWidget();
  _selection = new QWidget();
  _after = new QWidget();
  _splitter = new QSplitter(Qt::Horizontal, this);

  // mise en place du splitter
  _splitter->addWidget(_before);
  _splitter->addWidget(_selection);
  _splitter->addWidget(_after);
  _splitter->setHandleWidth(24);

  // changement des couleurs
  this->setBeforeColor(QColor(191, 191, 255));
  this->setSelectionColor(QColor(191, 255, 191));
  this->setAfterColor(QColor(191, 191, 255));

  // positionnement des éléments dans la widget
  QGridLayout *grid = new QGridLayout();
  grid->setMargin(0);
  grid->addWidget(_splitter, 0, 0);
  this->setLayout(grid);

  // connection des evènements
  QObject::connect(
    _splitter, SIGNAL(splitterMoved(int, int)),
    this, SLOT(updateValues(int, int))
    );
  QObject::connect(
    this, SIGNAL(minChanged(int)),
    this, SLOT(printVals(int))
    );
  QObject::connect(
    this, SIGNAL(maxChanged(int)),
    this, SLOT(printVals(int))
    );

  // valeurs par defaut
  _valueMin = 0;
  _valueMax = 100;
  _currentMin = 10;
  _currentMax = 90;
  this->updateMinPos();
  this->updateMaxPos();

  this->printVals();
}

// destructeur
QSlice::~QSlice()
{
}

// taille par defaut
QSize QSlice::sizeHint() const
{
  return QSize(480, 32);
}

// getters
int QSlice::currentMin()
{
  return qRound(_currentMin);
}

int QSlice::currentMax()
{
  return qRound(_currentMax);
}

int QSlice::valueMin()
{
  return (int)_valueMax;
}

int QSlice::valueMax()
{
  return (int)_valueMax;
}

//setters
void QSlice::setCurrentMin(int currentMin)
{
  if (currentMin >= _valueMin)
    _currentMin = currentMin;

  if (_currentMin > _currentMax)
    this->setCurrentMax(currentMin);

  this->updateMinPos();
}

void QSlice::setCurrentMax(int currentMax)
{
  if (currentMax <= _valueMax)
    _currentMax = currentMax;

  if (_currentMax < _currentMin)
    this->setCurrentMin(currentMax);

  this->updateMaxPos();
}

void QSlice::setValueMin(int valueMin)
{
  if (valueMin < _valueMax)
    _valueMin = valueMin;

  if (_currentMin < _valueMin)
    _currentMin = _valueMin;

  this->updateMinPos();
  this->updateMaxPos();
}

void QSlice::setValueMax(int valueMax)
{
  if (valueMax > _valueMin)
    _valueMax = valueMax;

  if (_currentMax > _valueMax)
    _currentMax = _valueMax;

  this->updateMinPos();
  this->updateMaxPos();
}

// couleur avant
void QSlice::setBeforeColor(const QColor &color)
{
  _before->setPalette(QPalette(color));
  _before->setAutoFillBackground(true);
}

// couleur séléction
void QSlice::setSelectionColor(const QColor &color)
{
  _selection->setPalette(QPalette(color));
  _selection->setAutoFillBackground(true);
}

// couleur après
void QSlice::setAfterColor(const QColor &color)
{
  _after->setPalette(QPalette(color));
  _after->setAutoFillBackground(true);
}

// recale le curseur du minimum
void QSlice::updateMinPos()
{
  // probleme de positions => marges?
  QSize size = _before->size();
  size.setWidth(qRound(
    (_currentMin - _valueMin) *
    (double)(_before->width() + _selection->width() + _after->width()) /
    (_valueMax - _valueMin)));
  _before->resize(size);
}

// recale le curseur du maximum
void QSlice::updateMaxPos()
{
  // probleme de positions => marges?
  QSize size = _after->size();
  size.setWidth(qRound(
    (_valueMax - _currentMax) *
    (double)(_before->width() + _selection->width() + _after->width()) /
    (_valueMax - _valueMin)));
  _after->resize(size);
}

// mise a jour des valeurs
void QSlice::updateValues(int pos, int index)
{
  if (index == 1)
  {
    _currentMin =
      (_valueMax - _valueMin) /
      (double)(this->width() - _splitter->handleWidth() * 2) *
      pos +
      _valueMin;
    emit minChanged(qRound(_currentMin));
    if (_currentMin > _currentMax)
    {
      _currentMax = _currentMin;
      emit maxChanged(qRound(_currentMax));
    }
  }
  if (index == 2)
  {
    _currentMax =
      (_valueMax - _valueMin) /
      (double)(this->width() - _splitter->handleWidth() * 2) *
      (pos - _splitter->handleWidth()) +
      _valueMin;
    emit maxChanged(qRound(_currentMax));
    if (_currentMax < _currentMin)
    {
      _currentMin = _currentMax;
      emit minChanged(qRound(_currentMin));
    }
  }
}

// affichage des changements
void QSlice::printVals(int /*val*/)
{
  qDebug() << "Slice [" << qRound(_currentMin)
    << "-" << qRound(_currentMax) << "] \t"
    << "Total [" << qRound(_valueMin)
    << "-" << qRound(_valueMax) << "]";
}
