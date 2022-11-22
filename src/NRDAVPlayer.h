#pragma once

#include <QtWidgets/QMainWindow>
#include <QTime>
#include "ui_NRDAVPlayer.h"
#include "Reader.h"

class AVMenuWidget;
class NRDAVPlayer : public QMainWindow
{
  Q_OBJECT

public:
  NRDAVPlayer(const QString &mediaFile, const QString &alias, QWidget *parent = Q_NULLPTR);

protected:
  void resizeEvent(QResizeEvent *e);
  void dragEnterEvent(QDragEnterEvent *evt);
  void dropEvent(QDropEvent *evt);
  bool LoadBxx(const QString &bxxFile, QString &mediaFile, int &inPoint, int &outPoint);
  bool SaveBxx(const QString &bxxFile, int inPoint, int outPoint);
  QString GetBoxmediaPath();

protected slots:
  void checkHide();
  void onOpen(const QString &mediaFile);
  void onClose();
  void onDurationUpdated(const M_AV_PROPS &AVProps, double duration);
  void onPositionUpdated(const M_TIME &time);
  void onTransportCommandChange(ETransportCommand command);
  void onAVPropsUpdated(const M_AV_PROPS &AVProps);
  void open(const QString &url);
  void minChanged(int pos);
  void maxChanged(int pos);

protected:
  AVMenuWidget *m_AVMenuWidget;
  Reader *m_reader;
  QTimer *m_hideTimer;
  struct { QPoint p; QTime t; } m_lastShow;
  bool m_editable;
  int m_inPoint, m_outPoint;
  QString m_bxxFile;
  bool m_autoPlay;
  QString m_aliasSuffix;

private:
  Ui::NRDAVPlayerClass ui;
};
