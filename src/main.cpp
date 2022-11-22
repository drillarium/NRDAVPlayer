#include "NRDAVPlayer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
  QString mediaFile, alias;
  if(argc > 1)
    mediaFile = argv[1];
  if(argc > 2)
    alias = argv[2];

  QApplication a(argc, argv);
  NRDAVPlayer w(mediaFile, alias);
  w.show();
  return a.exec();
}
