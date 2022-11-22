#pragma once

#include <QWidget>
#include "Reader.h"

class VumeterWidget : public QWidget
{
  Q_OBJECT

public:
  VumeterWidget(QWidget *parent = Q_NULLPTR);
  ~VumeterWidget();

  void Update(const M_AV_PROPS &avProps);
  QSize sizeHint() const;
  void Clear();

protected:
  void paintEvent(QPaintEvent *e);

protected:
  bool m_validAVProps;
  M_AV_PROPS m_lastAVProps;
};
