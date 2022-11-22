#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include "AVMenuWidget.h"
#include "Constants.h"

__inline void GetFrameRate(eMTimecodeFlags tcflags, int &num, int &den)
{
  if((tcflags & eMTCF_Rate_50) == eMTCF_Rate_50) { num = 50; den = 1; }
  else if((tcflags & eMTCF_Rate_60) == eMTCF_Rate_60) { num = 60000; den = 1001; }
  else if((tcflags & eMTCF_Rate_25) == eMTCF_Rate_25) { num = 25; den = 1; }
  else if((tcflags & eMTCF_Rate_30) == eMTCF_Rate_30) { num = 30000; den = 1001; }
}

QString TimeCode2String(const M_TIMECODE &tc)
{
  QString ret;

  int frNum = 0, frDen = 0;
  GetFrameRate(tc.eTCFlags, frNum, frDen);
  bool dropFrame = !!(tc.eTCFlags & eMTCF_DropFrame);
  QString delimiter = dropFrame ? ";" : ":";
  if(tc.eTCFlags & 0xF00)
  {
    int fieldNumber = ((tc.eTCFlags & 0xE00) >> 8);
    fieldNumber = qMax(0, fieldNumber - 1);
    ret = QString("%1:%2:%3%4%5.%6").arg(tc.nHours, 2, 10, QChar('0')).arg(tc.nMinutes, 2, 10, QChar('0')).arg(tc.nSeconds, 2, 10, QChar('0')).arg(delimiter).arg(tc.nFrames, 2, 10, QChar('0')).arg(fieldNumber, 2, 10, QChar('0'));
  }
  else
    ret = QString("%1:%2:%3%4%5").arg(tc.nHours, 2, 10, QChar('0')).arg(tc.nMinutes, 2, 10, QChar('0')).arg(tc.nSeconds, 2, 10, QChar('0')).arg(delimiter).arg(tc.nFrames, 2, 10, QChar('0'));

  return ret;
}

#define DBL2INT(dbl)		(INT)( ((dbl) >= 0.0)? (dbl) + 0.5 : (dbl) - 0.5 )

template <class TEnum>
inline TEnum PutFlag(TEnum* _peFlags, TEnum _eFlag)
{
  *_peFlags = (TEnum)((int)(*_peFlags) | _eFlag);
  return *_peFlags;
}

inline static eMTimecodeFlags Rate2Flags(int nRate, bool _bDrop = false, eMTimecodeFlags* _peFlags = NULL)
{
  eMTimecodeFlags eFlags = eMTCF_NonDropFrame;
  if(nRate > 45)
  {
    nRate /= 2;
    PutFlag(&eFlags, eMTCF_Progressive_Even_Frame);
  }
  if(nRate == 24)
    PutFlag(&eFlags, eMTCF_Rate_24);
  else if(nRate == 25)
    PutFlag(&eFlags, eMTCF_Rate_25);
  else if(nRate == 30)
    PutFlag(&eFlags, eMTCF_Rate_30);

  if(_bDrop && nRate == 30)
    PutFlag(&eFlags, eMTCF_DropFrame);

  if(_peFlags)
    PutFlag(_peFlags, eFlags);

  return eFlags;
}

QString Frames2TimeCodeString(int frames, double rate)
{
  M_TIMECODE tc = { 0 };

  if(frames < 0)
    tc.eTCFlags = eMTCF_Invalid;
  else
  {
    bool dropFrame = true;
    int nBase = DBL2INT(rate);
    if((nBase != 30 && nBase != 60) || (nBase - rate) < 0.005)
      dropFrame = false;

    if(nBase > 45)
    {
      // Progressive
      PutFlag(&tc.eTCFlags, (frames % 2) ? eMTCF_Progressive_Odd_Frame : eMTCF_Progressive_Even_Frame);
      nBase /= 2;
      frames /= 2;
    }

    // Put rate flags
    Rate2Flags(nBase, dropFrame, &tc.eTCFlags);

    if(nBase == 0)
      nBase = 30;

    // Convert from time to timecode
    int nSeconds = (int)(frames / nBase);
    if(dropFrame)
    {
      // Frames in ten minutes 600 * DBL2INT( dblRate ) - 20 + 2 (17982 for 29.97)
      int nFramesIn10Min = 600 * nBase - 10 * 2 + 2;
      int nTenMinutes = frames / nFramesIn10Min;
      frames %= nFramesIn10Min;
      // Frames in one minute 60 * DBL2INT( dblRate ) - 2  (1798 for 29.97, 1800 for first minute)
      if(frames < nBase * 60)
      {
        // First minute
        nSeconds = frames / nBase + nTenMinutes * 10 * 60;
        frames %= nBase;
      }
      else
      {
        // Next minutes (1798 frames in minute)
        int nMinutes = 1 + (frames - nBase * 60) / (nBase * 60 - 2);
        frames = (frames - nBase * 60) % (nBase * 60 - 2);
        // 28 frames in first second
        if(frames < nBase - 2)
        {
          // First second
          nSeconds = 60 * nMinutes + nTenMinutes * 10 * 60;
          frames += 2;
        }
        else
        {
          // Next seconds
          nSeconds = 60 * nMinutes + nTenMinutes * 10 * 60 + 1;
          nSeconds += (frames - (nBase - 2)) / nBase;
          frames = (frames - (nBase - 2)) % nBase;
        }
      }

      tc.nHours = nSeconds / 3600;
      tc.nMinutes = (((nSeconds % 3600) + 3600) % 3600) / 60;
      tc.nSeconds = ((nSeconds % 60) + 60) % 60;
      tc.nFrames = frames;
    }
    else
    {
      tc.nHours = nSeconds / 3600;
      tc.nMinutes = (((nSeconds % 3600) + 3600) % 3600) / 60;
      tc.nSeconds = ((nSeconds % 60) + 60) % 60;
      tc.nFrames = ((frames % nBase) + nBase) % nBase;
    }
  }

  return TimeCode2String(tc);
}

AVMenuWidget::AVMenuWidget(QWidget *parent)
:QWidget(parent)
,m_duration(0)
{
  ui.setupUi(this);

  ui.sliceWidget->setVisible(false);
  QSizePolicy sp_retain = ui.sliceWidget->sizePolicy();
  sp_retain.setRetainSizeWhenHidden(true);
  ui.sliceWidget->setSizePolicy(sp_retain);

  connect(ui.sliceWidget, &QSlice::minChanged, this, &AVMenuWidget::minChanged);
  connect(ui.sliceWidget, &QSlice::maxChanged, this, &AVMenuWidget::maxChanged);

  CreateEffectsMenu();
}

AVMenuWidget::~AVMenuWidget()
{

}

void AVMenuWidget::onEjectClicked()
{
  emit onEject();
}

void AVMenuWidget::onStopClicked()
{
  emit onStop();
}

void AVMenuWidget::onPlayClicked()
{
  if(m_lastTransportCommand == ETransportCommand::CMD_PLAY)
    emit onPause();
  else
    emit onPlay();
}

void AVMenuWidget::onFFClicked()
{
  emit onFF();
}

void AVMenuWidget::onFRClicked()
{
  emit onFR();
}

void AVMenuWidget::onUpdatePosition(int position)
{
  emit onSeek(position);
}

bool AVMenuWidget::Clear()
{
  ui.positionSlider->setMinimum(0);
  ui.positionSlider->setMaximum(100);
  ui.positionSlider->setSliderPosition(0);
  ui.positionSlider->setDisabled(true);
  ui.positionLabel->setText("00:00:00:00");
  ui.durationLabel->setText("00:00:00:00");
  SetTransportCommand(ETransportCommand::CMD_NONE);
  ui.sliceWidget->setVisible(false);

  return true;
}

bool AVMenuWidget::SetDuration(const M_AV_PROPS &AVProps, double &duration)
{
  m_duration = (int) (duration * AVProps.vidProps.dblRate);
  ui.positionSlider->setMinimum(0);
  ui.positionSlider->setMaximum(m_duration);
  ui.positionSlider->setSliderPosition(0);
  ui.positionSlider->setDisabled(false);
  QString tc = Frames2TimeCodeString(m_duration, AVProps.vidProps.dblRate);
  ui.durationLabel->setText(tc);

  ui.fastRewindButton->setEnabled(m_duration != 0);
  ui.playButton->setEnabled((m_lastTransportCommand != ETransportCommand::CMD_PLAY) || (m_duration != 0));
  ui.fastForwardButton->setEnabled(m_duration != 0);

  return true;
}

bool AVMenuWidget::SetInOutPoints(bool editable, int in, int out)
{
  ui.sliceWidget->setVisible(editable);
  ui.sliceWidget->setMinMax(0, m_duration, in, out);

  return true;
}

bool AVMenuWidget::SetPosition(const M_TIME &time)
{
  ui.positionSlider->setSliderPosition(time.tcFrame.nExtraCounter);
  ui.positionLabel->setText(TimeCode2String(time.tcFrame));

  return true;
}

bool AVMenuWidget::SetTransportCommand(ETransportCommand command)
{
  switch(command)
  {
    case ETransportCommand::CMD_PLAY:
      ui.playButton->setIcon(QIcon(":/pause.svg"));
    break;
    default:
      ui.playButton->setIcon(QIcon(":/play.svg"));
    break;
  }

  m_lastTransportCommand = command;
  ui.playButton->setEnabled((m_lastTransportCommand != ETransportCommand::CMD_PLAY) || (m_duration != 0));

  return true;
}

void AVMenuWidget::SetUrl(const QString &url)
{
  m_lastUrl = url;
}

void AVMenuWidget::onUpdateVolume(int pos)
{
  double fPos = ((double) pos) / (ui.volumeSlider->maximum() - ui.volumeSlider->minimum());
  emit onVolume(fPos);

  if(pos == 0)
    ui.volumeButton->setIcon(QIcon(":/mute.svg"));
  else
    ui.volumeButton->setIcon(QIcon(":/volume.svg"));
}

void AVMenuWidget::onInputUrlClicked()
{
  bool ok;
  QString text = QInputDialog::getText(this, "NRDAVPlayer", "Url:", QLineEdit::Normal, m_lastUrl, &ok, (this->windowFlags() | Qt::FramelessWindowHint));
  if(ok && !text.isEmpty())  
    emit openUrl(text);
}

void AVMenuWidget::onAbout()
{
  QMessageBox::information(this, "NRDAVPlayer", QString("Version: %1").arg(APP_VERSION), QMessageBox::StandardButton::Ok);
}

void AVMenuWidget::onMuteClicked()
{
  ui.volumeSlider->setSliderPosition(0);
  onUpdateVolume(0);
}

void AVMenuWidget::updateRendererList(const QStringList &renderers)
{
  QMenu *menu = new QMenu(this);
  foreach(auto e, renderers)
  {
    QAction *action = new QAction(e, menu);
    action->setCheckable(true),
    menu->addAction(action);
  }

  connect(menu, &QMenu::triggered, this, &AVMenuWidget::onRendererClicked);

  ui.outputButton->setMenu(menu);
}

void AVMenuWidget::onRendererClicked(QAction *action)
{
  bool checked = action->isChecked();
  QString name = action->text();

  emit onRendererSelected(!checked? "" : name);
}

void AVMenuWidget::onNewRenderer(const QString &renderer)
{
  QMenu *menu = ui.outputButton->menu();
  if(!menu)
    return;

  QList<QAction *> al = menu->actions();
  foreach(auto e, al)
  {
    QString name = e->text();
    e->setChecked(e->text() == renderer);
  }    
}

void AVMenuWidget::CreateEffectsMenu()
{
  QMenu *menu = new QMenu(this);
  QAction *action = new QAction(EFFECT_VER_REFL, menu);
  action->setCheckable(true),
  menu->addAction(action);
  action = new QAction(EFFECT_HOR_REFL, menu);
  action->setCheckable(true),
  menu->addAction(action);
  action = new QAction(EFFECT_BOTH_REFL, menu);
  action->setCheckable(true),
  menu->addAction(action);

  connect(menu, &QMenu::triggered, this, &AVMenuWidget::onEffectClicked);

  ui.effectButton->setMenu(menu);
}

void AVMenuWidget::onEffectClicked(QAction *action)
{
  bool checked = action->isChecked();
  QString name = action->text();

  emit onEffect(!checked ? "" : name);
}

void AVMenuWidget::onNewEffect(const QString &effect)
{
  QMenu *menu = ui.effectButton->menu();
  if(!menu)
    return;

  QList<QAction *> al = menu->actions();
  foreach(auto e, al)
    e->setChecked(false);

  if(effect == EFFECT_VER_REFL)
    al[0]->setChecked(true);
  else if(effect == EFFECT_HOR_REFL)
    al[1]->setChecked(true);
  else if(effect == EFFECT_BOTH_REFL)
    al[2]->setChecked(true);
}
