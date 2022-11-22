#include <QWidget>
#include "Reader.h"
//#include "MLProtect_MFormats SDK.h"
#include "MLProtect_MFormats SDK.(subscription_valid_until_25_May_2023).h"
#include "Constants.h"

Reader::Reader(LONGLONG videoWindow, QObject *parent)
:QThread(parent)
,m_running(false)
,m_videoWindow(videoWindow)
,m_preview(NULL)
,m_fSpeed(1)
{  
  qRegisterMetaType<M_TIME>("M_TIME");
  qRegisterMetaType<M_AV_PROPS>("M_AV_PROPS");
  qRegisterMetaType<ETransportCommand>("ETransportCommand");

  MFormatsSDKLic::IntializeProtection();

  start();
}

Reader::~Reader()
{
  if(m_running)
  {
    m_running = false;
    wait();
  }
}

void Reader::run()
{
  HRESULT hrConInit = ::CoInitialize(NULL);

  m_running = true;

  CComPtr<IMFFrame> blackFrame;
  CComPtr<IMFFactory> factory;
  HRESULT hr = factory.CoCreateInstance(__uuidof(MFFactory));
  if(SUCCEEDED(hr))
  {
    CComBSTR colorParameters = L"solid_color='Black(255)'";
    M_AV_PROPS avProps = {};
    avProps.vidProps.eVideoFormat = eMVF_PAL;
    factory->MFFrameCreateFromMem(&avProps, (LONGLONG) 0, (long) 0, (LONGLONG) 0, &blackFrame, colorParameters);
  }

  hr = m_preview.CoCreateInstance(__uuidof(MFPreview));
  if(SUCCEEDED(hr))
  {
    CComBSTR channelID;
    BOOL enableVideo = TRUE;
    BOOL enableAudio = TRUE;
    hr = m_preview->PreviewEnable(channelID, enableAudio, enableVideo);
    hr = m_preview->PreviewWindowSet(channelID, m_videoWindow);
  }

  CComPtr<IMFReader> reader;
  hr = reader.CoCreateInstance(__uuidof(MFReader));
  bool opened = false;
  bool reopen = false;
  int nextFrame = -1;
  int curFrame = 0;

  m_live.CoCreateInstance(__uuidof(MFRenderer));
  GetDeviceList();  

  bool loop = false;

  while(m_running)
  {
    // reader
    if( (m_nextUrl != m_currentUrl) || reopen)
    {
      if(opened)
      {
        if(reader)
          reader->ReaderClose();
        opened = false;

        emit onClose();
      }

      bool success = m_nextUrl.isEmpty();
      if(!m_nextUrl.isEmpty())
      {
        // Mirror
        CComQIPtr<IMProps> props(reader);
        if(m_nextEffect == EFFECT_VER_REFL)
          props->PropsSet((BSTR) L"mirror", (BSTR) L"vert");
        else if(m_nextEffect == EFFECT_HOR_REFL)
          props->PropsSet((BSTR) L"mirror", (BSTR) L"horz");
        else if(m_nextEffect == EFFECT_BOTH_REFL)
          props->PropsSet((BSTR) L"mirror", (BSTR) L"both");

        emit onNewEffect(m_nextEffect);

        CComBSTR filePath = m_nextUrl.toStdWString().c_str();
        CComBSTR propList;
        success = SUCCEEDED(reader->ReaderOpen(filePath, propList));
        if(success)
        {
          opened = true;

          emit onOpen(m_nextUrl);

          double duration = 0;
          reader->ReaderDurationGet(&duration);
          M_AV_PROPS avProps = {};
          reader->SourceAVPropsGet(&avProps);
          emit onDurationUpdated(avProps, duration);
        }
      }

      if(success)
      {
        reopen = false;
        m_currentUrl = m_nextUrl;

        // reset transport commands
        m_nextCommand = { ETransportCommand::CMD_STOP };
        curFrame = 0;
        nextFrame = -1;
        m_fSpeed = 1;
      }
    }

    // renderer
    if(m_nextRenderer != m_currentRenderer)
    {
      OpenRenderer(m_nextRenderer);
      m_currentRenderer = m_nextRenderer;
    }

    // Get frame from reader
    CComPtr<IMFFrame> frame;
    CComQIPtr<IMFSource> source(reader);
    if(source && opened)
    {
      // command to process
      if(m_nextCommand.command != CMD_NONE)
      {
        if(m_nextCommand.command == CMD_STOP)
          nextFrame = 0;
        else if(m_nextCommand.command == CMD_PLAY)
        {
          if(loop)
            nextFrame = 0;
          else
            nextFrame = -1;
        }          
        else if(m_nextCommand.command == CMD_PAUSE)
          nextFrame = curFrame;
        else if(m_nextCommand.command == CMD_SEEK)
          nextFrame = m_nextCommand.arg;

        bool emit_ = (m_nextCommand.command != m_command.command);
        m_command = m_nextCommand;
        m_nextCommand = { CMD_NONE };
        if(emit_)
          emit onTransportCommandChange(m_command.command);
      }

      int frameN = nextFrame;
      REFERENCE_TIME maxWait = -1;
      CComBSTR props;
      hr = source->SourceFrameGetByNumber(frameN, maxWait, &frame, props);
      // if(hr == 0x8007054b)  // TESTING: no fer res. Quan mp esta obert però no estem escribint res
      if(hr == 0x800706ba)  // TESTING: cal reconectar. Per exemple al tancar i obrir de nou engine (UNAVAILABLE)      
        reopen = true;

      // FF FR
      if(m_command.command == CMD_FF)
      {
        nextFrame = curFrame + (2 * m_fSpeed);
        // if(nextFrame >= )
          // nextFrame = - 1;
      }
      else if(m_command.command == CMD_FR)
      {
        nextFrame = curFrame - (2 * m_fSpeed);
        if(nextFrame < 0)
          nextFrame = 0;
      }

      // notify
      if(SUCCEEDED(hr))
      {
        M_TIME t;
        frame->MFTimeGet(&t);
        emit onPositionUpdated(t);

        M_AV_PROPS avProps = {};
        frame->MFAVPropsGet(&avProps, NULL);
        emit onAVPropsUpdated(avProps);

        // cur frame
        curFrame = t.tcFrame.nExtraCounter;

        // last
        bool last = !!(t.eFFlags & eMFF_Last);        
        if(last)
          m_nextCommand = { CMD_PAUSE };
        // proves loop
        if(loop)
          nextFrame = -1;
        loop = last; 
        if(loop)
          m_nextCommand = { CMD_PLAY };
      }
    }

    // renderer
    if(!m_currentRenderer.isEmpty() && m_live)
    {
      CComQIPtr<IMFReceiver> preview(m_live);
      REFERENCE_TIME maxWait = 0;
      CComBSTR props;
      hr = preview->ReceiverFramePut(frame ? frame : blackFrame, maxWait, props);
    }

    // sink
    CComQIPtr<IMFReceiver> preview(m_preview);
    REFERENCE_TIME maxWait = -1;
    CComBSTR props;
    hr = preview->ReceiverFramePut(frame? frame : blackFrame, maxWait, props);
  }

  if(reader)
  {
    reader->ReaderClose();
    reader.Release();
  }

  if(factory)
    factory.Release();

  if(blackFrame)
    blackFrame.Release();

  if(m_preview)
    m_preview.Release();

  if(SUCCEEDED(hrConInit))
    CoUninitialize();
}

bool Reader::UpdateVideoWindow()
{
  if(!m_preview)
    return false;

  CComBSTR channel;
  HRESULT hr = m_preview->PreviewWindowSet(channel, m_videoWindow);

  return SUCCEEDED(hr);
}

void Reader::open(const QString &url)
{
  m_nextUrl = url;
}

void Reader::close()
{
  m_nextUrl.clear();
}

void Reader::stop()
{
  m_nextCommand = { CMD_STOP };
}

void Reader::pause()
{
  m_nextCommand = { CMD_PAUSE };
}

void Reader::play()
{
  m_nextCommand = { CMD_PLAY };
}

void Reader::seek(int frame)
{
  m_nextCommand = { CMD_SEEK, frame };
}

void Reader::ff()
{  
  if(m_command.command == CMD_FR)
    m_fSpeed = 1;
  else
    m_fSpeed++;
  m_nextCommand = { CMD_FF };
}

void Reader::fr()
{
  if(m_command.command == CMD_FF)
    m_fSpeed = 1;
  else
    m_fSpeed++;
  m_nextCommand = { CMD_FR };
}

void Reader::volume(double pos)
{
  CComBSTR channelID;
  int channel = -1; // all
  double attenuation = -30 * (1 - pos);
  m_preview->PreviewAudioVolumeSet(channelID, channel, attenuation);
  
  BOOL enableAudio = TRUE;
  BOOL enableVideo = TRUE;
  m_preview->PreviewIsEnabled(channelID, &enableAudio, &enableVideo);
  enableAudio = pos > 0;
  m_preview->PreviewEnable(channelID, enableAudio, enableVideo);
}

void Reader::GetDeviceList()
{
  if(!m_live)
    return;

  QStringList l;
  int deviceCount = 0;
  m_live->DeviceGetCount(eMFDT_Video, &deviceCount);

  CComBSTR name;
  BOOL busy = FALSE;
  for(int i = 0; i < deviceCount; i++)
  {
    m_live->DeviceGetByIndex(eMFDT_Video, i, &name, &busy);
    if(name)    
      l += QString::fromUtf16((const ushort *) (LPWSTR) name);    
  }

  emit onRendererOpened(l);
}

void Reader::onRendererSelected(const QString &renderer)
{
  if(!m_live)
    return;

  if(renderer.isEmpty())
  {
    m_nextRenderer.clear();
    return;
  }

  int deviceCount = 0;
  m_live->DeviceGetCount(eMFDT_Video, &deviceCount);

  for(int i = 0; i < deviceCount; i++)
  {
    CComBSTR name;
    BOOL busy = FALSE;
    m_live->DeviceGetByIndex(eMFDT_Video, i, &name, &busy);
    if(name)
    {
      QString aux = QString::fromUtf16((const ushort *)(LPWSTR) name);
      if(aux == renderer)
      {
        m_nextRenderer = aux;
        break;
      }
    }
  }
}

void Reader::OpenRenderer(const QString &renderer)
{
  if(!m_live)
    return;

  int deviceCount = 0;
  m_live->DeviceGetCount(eMFDT_Video, &deviceCount);

  bool found = false;
  for(int i = 0; i < deviceCount && !found; i++)
  {
    CComBSTR name;
    BOOL busy = FALSE;
    m_live->DeviceGetByIndex(eMFDT_Video, i, &name, &busy);
    if(name)
    {
      QString aux = QString::fromUtf16((const ushort *)(LPWSTR) name);
      found = (aux == renderer);
      if(found)
      {
        emit onNewRenderer(renderer);

        CComBSTR params;
        m_live->DeviceSet(eMFDT_Video, i, params);
        break;
      }
    }
  }

  if(!found)
  {
    emit onNewRenderer("");

    CComBSTR params;
    m_live->DeviceSet(eMFDT_Video, -1, params);
  }
}

void Reader::onEffect(const QString &effect)
{
  m_nextEffect = effect;
}
