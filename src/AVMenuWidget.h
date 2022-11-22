#pragma once

#include <QWidget>
#include "ui_AVMenuWidget.h"
#include "Reader.h"

class AVMenuWidget : public QWidget
{
  Q_OBJECT

public:
  AVMenuWidget(QWidget *parent = Q_NULLPTR);
  ~AVMenuWidget();

  bool SetDuration(const M_AV_PROPS &AVProps, double &duration);
  bool SetPosition(const M_TIME &time);
  bool Clear();
  bool SetTransportCommand(ETransportCommand command);
  bool SetInOutPoints(bool editable, int in, int out);
  void SetUrl(const QString &url);

signals:
  void onEject();
  void onStop();
  void onPause();
  void onPlay();
  void onSeek(int);  
  void onFF();
  void onFR();
  void onVolume(double);
  void openUrl(const QString &);
  void minChanged(int value);
  void maxChanged(int value);
  void onRendererSelected(const QString &renderer);
  void onEffect(const QString &effect);

public slots:
  void updateRendererList(const QStringList &renderers);
  void onNewRenderer(const QString &renderer);  
  void onNewEffect(const QString &effect);

protected slots:
  void onEjectClicked();
  void onStopClicked();
  void onPlayClicked();
  void onFFClicked();
  void onFRClicked();
  void onUpdatePosition(int position);
  void onUpdateVolume(int pos);
  void onInputUrlClicked();
  void onAbout();
  void onMuteClicked();  
  void onRendererClicked(QAction *action);
  void onEffectClicked(QAction *action);

protected:
  void CreateEffectsMenu();

protected:
  ETransportCommand m_lastTransportCommand;
  int m_duration;
  QString m_lastUrl;

private:
  Ui::AVMenuWidget ui;
};
