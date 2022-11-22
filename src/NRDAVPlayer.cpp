#include <QTimer>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDomDocument>
#include <QSettings>
#include <QDir>
#include <QTextStream>
#include "NRDAVPlayer.h"
#include "AVMenuWidget.h"
#include "Reader.h"
#include "Constants.h"

NRDAVPlayer::NRDAVPlayer(const QString &mediaFile, const QString &alias, QWidget *parent)
:QMainWindow(parent)
,m_editable(false)
,m_autoPlay(false)
{
  ui.setupUi(this);

  if(!alias.isEmpty())
    m_aliasSuffix = QString(" - %1").arg(alias);
  setWindowTitle(QString("NRDAVPlayer - %1%2").arg(APP_VERSION).arg(m_aliasSuffix));
  QIcon icon(":/home.ico");
  setWindowIcon(icon);

  m_reader = new Reader(ui.label->winId(), this);
  m_AVMenuWidget = new AVMenuWidget(this);
  connect(m_reader, &Reader::onRendererOpened, m_AVMenuWidget, &AVMenuWidget::updateRendererList);

  connect(m_reader, &Reader::onOpen, this, &NRDAVPlayer::onOpen);
  connect(m_reader, &Reader::onClose, this, &NRDAVPlayer::onClose);
  connect(m_reader, &Reader::onDurationUpdated, this, &NRDAVPlayer::onDurationUpdated);
  connect(m_reader, &Reader::onPositionUpdated, this, &NRDAVPlayer::onPositionUpdated);
  connect(m_reader, &Reader::onTransportCommandChange, this, &NRDAVPlayer::onTransportCommandChange);
  connect(m_reader, &Reader::onAVPropsUpdated, this, &NRDAVPlayer::onAVPropsUpdated);
  
  connect(m_AVMenuWidget, &AVMenuWidget::onEject, m_reader, &Reader::close);
  connect(m_AVMenuWidget, &AVMenuWidget::onStop, m_reader, &Reader::stop);
  connect(m_AVMenuWidget, &AVMenuWidget::onPause, m_reader, &Reader::pause);
  connect(m_AVMenuWidget, &AVMenuWidget::onPlay, m_reader, &Reader::play);
  connect(m_AVMenuWidget, &AVMenuWidget::onSeek, m_reader, &Reader::seek);
  connect(m_AVMenuWidget, &AVMenuWidget::onFF, m_reader, &Reader::ff);
  connect(m_AVMenuWidget, &AVMenuWidget::onFR, m_reader, &Reader::fr);
  connect(m_AVMenuWidget, &AVMenuWidget::onVolume, m_reader, &Reader::volume);
  connect(m_AVMenuWidget, &AVMenuWidget::openUrl, this, &NRDAVPlayer::open);
  connect(m_AVMenuWidget, &AVMenuWidget::minChanged, this, &NRDAVPlayer::minChanged);
  connect(m_AVMenuWidget, &AVMenuWidget::maxChanged, this, &NRDAVPlayer::maxChanged);  
  connect(m_AVMenuWidget, &AVMenuWidget::onRendererSelected, m_reader, &Reader::onRendererSelected);
  connect(m_reader, &Reader::onNewRenderer, m_AVMenuWidget, &AVMenuWidget::onNewRenderer);
  connect(m_AVMenuWidget, &AVMenuWidget::onEffect, m_reader, &Reader::onEffect);
  connect(m_reader, &Reader::onNewEffect, m_AVMenuWidget, &AVMenuWidget::onNewEffect);
  
  m_hideTimer = new QTimer(this);
  connect(m_hideTimer, &QTimer::timeout, this, &NRDAVPlayer::checkHide);
  m_hideTimer->start(1000);

  setAcceptDrops(true);

  onClose();

  if(!mediaFile.isEmpty())
  {
    open(mediaFile);

    QString scheme = QUrl(mediaFile).scheme();
    if(!scheme.isEmpty() && scheme.compare("file", Qt::CaseInsensitive))
      m_autoPlay = true;
  }
}

void NRDAVPlayer::checkHide()
{
  QRect r = geometry();
  QRect rAVMenu = m_AVMenuWidget->geometry();
  QPoint tl = mapToGlobal(rAVMenu.topLeft());
  rAVMenu = QRect(tl.x(), tl.y(), rAVMenu.width(), rAVMenu.height());
  QPoint p = QCursor::pos();

  bool hide = false, show = false;
  hide = !r.contains(p);
  show = rAVMenu.contains(p);
  if(!hide && !show)
  {
    bool moved = m_lastShow.p != p;
    if(m_AVMenuWidget->isVisible())
    {
      if(!moved)
      {
        if(QTime::currentTime() > m_lastShow.t.addMSecs(1000))
          hide = true;
      }
      else
        m_lastShow.p = p;
    }
    else
    {
      if(moved)
        show = true;
    }
  }

  if(show)
  {
    m_AVMenuWidget->show();
    m_lastShow.p = p;
    m_lastShow.t = QTime::currentTime();
  }
  else if(hide)  
    m_AVMenuWidget->hide();  
}

void NRDAVPlayer::resizeEvent(QResizeEvent *e)
{
  QSize s = size();
  int x = (s.width() >> 1) - (m_AVMenuWidget->width() >> 1);
  m_AVMenuWidget->setGeometry(x, s.height() - m_AVMenuWidget->height() - 20, m_AVMenuWidget->width(), m_AVMenuWidget->height());
  m_reader->UpdateVideoWindow();

  QMainWindow::resizeEvent(e);
}

void NRDAVPlayer::dragEnterEvent(QDragEnterEvent *evt)
{
  if(evt->mimeData()->hasUrls())
    evt->accept();
}

void NRDAVPlayer::dropEvent(QDropEvent *evt)
{
  auto urls = evt->mimeData()->urls();
  foreach(auto url, urls)
  {
    open(url.toLocalFile());    
    break;
  }
}

void NRDAVPlayer::open(const QString &_url)
{
  bool open = true;
  QString url(_url);  

  /* check extention */
  QFileInfo fi(url);
  QString extension = fi.suffix();
  m_editable = !extension.compare("bxx", Qt::CaseInsensitive);
  if(m_editable)
  {
    m_bxxFile = url;
    
    /* first media file and trim points */
    open = LoadBxx(m_bxxFile, url, m_inPoint, m_outPoint);
  }

  m_AVMenuWidget->SetUrl(_url);
  m_autoPlay = false;
  if(open)
    m_reader->open(url);  
}

void NRDAVPlayer::onOpen(const QString &mediaFile)
{
  setWindowTitle(QString("NRDAVPlayer - %1 - %2%3").arg(APP_VERSION).arg(mediaFile).arg(m_aliasSuffix));
  ui.stackedWidget->setCurrentIndex(0);
}

void NRDAVPlayer::onClose()
{
  setWindowTitle(QString("NRDAVPlayer - %1%2").arg(APP_VERSION).arg(m_aliasSuffix));
  ui.stackedWidget->setCurrentIndex(1);

  m_AVMenuWidget->Clear();
  ui.vumeterWidget->Clear();
}

void NRDAVPlayer::onDurationUpdated(const M_AV_PROPS &AVProps, double duration)
{
  m_reader->UpdateVideoWindow();
  if(m_autoPlay)
  {
    m_reader->play();
    m_autoPlay = false;
  }
  m_AVMenuWidget->SetDuration(AVProps, duration);
  m_AVMenuWidget->SetInOutPoints(m_editable, m_inPoint, m_outPoint);
}

void NRDAVPlayer::onPositionUpdated(const M_TIME &time)
{
  m_AVMenuWidget->SetPosition(time);
}

void NRDAVPlayer::onTransportCommandChange(ETransportCommand command)
{
  m_AVMenuWidget->SetTransportCommand(command);
}

void NRDAVPlayer::onAVPropsUpdated(const M_AV_PROPS &AVProps)
{
  ui.vumeterWidget->Update(AVProps);
}

void NRDAVPlayer::minChanged(int pos)
{
  m_reader->seek(pos);
  m_inPoint = pos;
  SaveBxx(m_bxxFile, m_inPoint, m_outPoint);
}

void NRDAVPlayer::maxChanged(int pos)
{
  m_reader->seek(pos);
  m_outPoint = pos;
  SaveBxx(m_bxxFile, m_inPoint, m_outPoint);
}

bool NRDAVPlayer::LoadBxx(const QString &bxxFile, QString &mediaFile, int &inPoint, int &outPoint)
{
  QFile f(bxxFile);
  if(!f.open(QFile::ReadOnly | QIODevice::Text))
    return false;

  QString boxmediaPath = GetBoxmediaPath();

  QDomDocument xmlFile;
  if(xmlFile.setContent(&f))
  {
    QDomNode root = xmlFile.documentElement();
    QDomElement vs = root.firstChildElement("VideoStream");
    while(!vs.isNull())
    {
      QDomElement vse = vs.firstChildElement("VideoStreamElement");
      while(!vse.isNull())
      {
        mediaFile = vse.firstChildElement("Name").toElement().text();
        inPoint = vse.firstChildElement("FileTrimIn").toElement().text().toInt();
        outPoint = vse.firstChildElement("FileTrimOut").toElement().text().toInt();                

        /* absolute path */
        mediaFile = QDir::cleanPath(boxmediaPath + QDir::separator() + mediaFile);

        break;

        vse = vse.nextSiblingElement("VideoStreamElement");
      }
      break;

      vs = vs.nextSiblingElement("VideoStream");
    }
  }

  f.close();

  return true;
}

bool SetNodeText(QDomNode node, const QString &text)
{
  bool valid = false;
  QDomNode childNode = node.firstChild();
  while(!childNode.isNull() && !valid)
  {
    valid = (childNode.nodeType() == QDomNode::TextNode);
    if(valid)
      childNode.setNodeValue(text);
  }

  if(!valid)
    node.appendChild(node.ownerDocument().createTextNode(text));

  return true;
}

bool NRDAVPlayer::SaveBxx(const QString &bxxFile, int inPoint, int outPoint)
{
  QFile f(bxxFile);
  if(!f.open(QFile::ReadOnly | QIODevice::Text))
    return false;

  QDomDocument xmlFile;
  QDomNode root;
  bool ret = xmlFile.setContent(&f);
  f.close();
  if(ret)
  {
    int newDuration = outPoint - inPoint;

    root = xmlFile.documentElement();
    SetNodeText(root.firstChildElement("Duration"), QString::number(newDuration));

    QDomElement vs = root.firstChildElement("VideoStream");
    while(!vs.isNull())
    {
      SetNodeText(vs.firstChildElement("Duration"), QString::number(newDuration));
      QDomElement vse = vs.firstChildElement("VideoStreamElement");
      while(!vse.isNull())
      {
        SetNodeText(vse.firstChildElement("FileTrimIn"), QString::number(inPoint));
        SetNodeText(vse.firstChildElement("FileTrimOut"), QString::number(outPoint));

        break;

        vse = vse.nextSiblingElement("VideoStreamElement");
      }
      break;

      vs = vs.nextSiblingElement("VideoStream");
    }
  }  

  if(ret)
  {
    ret = f.open(QIODevice::WriteOnly | QIODevice::Text);
    if(ret)
    {
      QTextStream textStream(&f);
      char encoding[] = "UTF-8";
      textStream.setCodec(encoding);

      QDomProcessingInstruction header = QDomDocument().createProcessingInstruction("xml", QString("version=\"1.0\" encoding=\"%1\"").arg(encoding));
      header.save(textStream, 4 /*indent*/);

      root.save(textStream, 4 /*indent*/);

      f.close();
    }
  }

  return ret;
}

QString NRDAVPlayer::GetBoxmediaPath()
{
  QString path = QDir::cleanPath(QDir::currentPath() + QDir::separator() + "NRDAVPLayer.ini");
  QSettings settings(path, QSettings::IniFormat);
  QString boxmedia = settings.value("boxmedia", "D:\\Boxmedia").toString();
  settings.setValue("boxmedia", boxmedia);
  settings.sync();

  return boxmedia;
}
