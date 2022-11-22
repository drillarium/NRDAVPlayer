#pragma once

#include <QThread>
#include <atlbase.h>    /* CComPtr */
#include "MFormats.h"   /* MFormats */

enum ETransportCommand { CMD_NONE, CMD_STOP, CMD_PLAY, CMD_PAUSE, CMD_SEEK, CMD_FF, CMD_FR };
struct STransportCommand { ETransportCommand command; int arg; };

class Reader : public QThread
{
  Q_OBJECT

public:
  Reader(LONGLONG videoWindow, QObject *parent);
  ~Reader();

  bool UpdateVideoWindow();    

protected:
  void run();  
  void GetDeviceList();
  void OpenRenderer(const QString &renderer);

public slots:
  void open(const QString &url);
  void close();
  void stop();
  void pause();
  void play();
  void seek(int frame);
  void ff();
  void fr();
  void volume(double pos);
  void onRendererSelected(const QString &renderer);
  void onEffect(const QString &effect);

signals:
  void onOpen(const QString &mediaFile);
  void onClose();
  void onDurationUpdated(const M_AV_PROPS &AVProps, double duration);
  void onPositionUpdated(const M_TIME &time);
  void onTransportCommandChange(ETransportCommand command);
  void onAVPropsUpdated(const M_AV_PROPS &avProps);
  void onRendererOpened(const QStringList &renderers);
  void onNewRenderer(const QString &renderer);
  void onNewEffect(const QString &renderer);

protected:
  CComPtr<IMPreview> m_preview;
  bool m_running;
  LONGLONG m_videoWindow;
  QString m_nextUrl;
  QString m_currentUrl;
  STransportCommand m_nextCommand;
  STransportCommand m_command;
  int m_fSpeed;
  CComPtr<IMFDevice> m_live;
  QString m_nextRenderer;
  QString m_currentRenderer;
  QString m_nextEffect;
};
