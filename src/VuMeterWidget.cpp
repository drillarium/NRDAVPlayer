#include <QTimer>
#include <QPainter>
#include "VumeterWidget.h"

#define BAR_WIDTH 4
#define NUM_CHANNELS 16

VumeterWidget::VumeterWidget(QWidget *parent)
:QWidget(parent)
,m_validAVProps(false)
{

}

VumeterWidget::~VumeterWidget()
{

}

QSize VumeterWidget::sizeHint() const
{
  int w = BAR_WIDTH * NUM_CHANNELS;
  int h = height();

  return QSize(w, h);
}

void VumeterWidget::Update(const M_AV_PROPS &avProps)
{
  m_lastAVProps = avProps;
  m_validAVProps = true;

  QTimer::singleShot(0, this, SLOT(repaint()));
}

void VumeterWidget::Clear()
{
  M_AV_PROPS avProps = {};
  Update(avProps);
}

bool is_infinite(float value)
{
  float max_value = (std::numeric_limits<float>::max)(); // ( ) prevent macro expansion https://stackoverflow.com/questions/27442885/syntax-error-with-stdnumeric-limitsmax
  float min_value = -max_value;
  return !(min_value <= value) && (value <= max_value);
}

void VumeterWidget::paintEvent(QPaintEvent *e)
{
  QPainter painter(this);

  int width_ = width();
  int height_ = height();

  QRect r(0, 0, width_, height_);
  painter.fillRect(r, "#000000");

  if(m_validAVProps)
  {
    QLinearGradient gradient(r.topLeft(), r.bottomRight());
    gradient.setColorAt(1, Qt::green);
    gradient.setColorAt(0.2, Qt::yellow);
    gradient.setColorAt(0, Qt::red);

    int numChannels = m_lastAVProps.ancData.audOriginal.nValidChannels;
    int w = BAR_WIDTH;
    QRect vumR(0, 0, w - 1, height_);
    for(int i = 0; i < numChannels; i++)
    {
      bool valid = !is_infinite(m_lastAVProps.ancData.audOriginal.arrVUMeter[i]);
      float max = 100.;
      float v = max + (valid ? m_lastAVProps.ancData.audOriginal.arrVUMeter[i] : (1 - max));
      vumR.setTop(vumR.bottom() - ((v * height_) / max));

      painter.fillRect(vumR, gradient);

      vumR.setX(vumR.x() + w);
      vumR.setWidth(w - 1);
    }
  }
}
