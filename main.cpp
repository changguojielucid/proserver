#include "EVFirst.h"
#include "EVProServer.h"
#include "EVWorkClient.h"
#include <QApplication>
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>


void PrintListOfTargets(EVWorkItem* pWorkItem)
 {
  std::cout << "Getting List of Targets..." << std::endl;
  
  if (pWorkItem->GetTargetDefs()->empty()) {
    std::cout << "List of Targets is empty..." << std::endl;
  }
  else {
    std::cout << "List of Targets is not empty..." << std::endl;
  }
}

int main(int argc, char *argv[])
{
 
  QApplication app(argc, argv);
  
  /*
  EVWorkClient client;
  EVWorkItem* pWorkItem = client.CreateWorkItem("D:/vt/DICOMS/ex20515/1/A", true);

  EVTargetDefine* pEvTargetDefine = pWorkItem->GetTargetDefine();

  PrintListOfTargets(pWorkItem);
 
  pEvTargetDefine->EstablishNewTarget("LeftCarotid");

  PrintListOfTargets(pWorkItem);
  */

  QCommandLineParser parser;
  parser.setApplicationDescription("QtWebSockets example: echoserver");
  parser.addHelpOption();

  QCommandLineOption dbgOption(QStringList() << "d" << "debug",
	  QCoreApplication::translate("main", "Debug output [default: off]."));
  parser.addOption(dbgOption);
  QCommandLineOption portOption(QStringList() << "p" << "port",
	  QCoreApplication::translate("main", "Port for echoserver [default: 1234]."),
	  QCoreApplication::translate("main", "port"), QLatin1String("1234"));
  parser.addOption(portOption);
  parser.process(app);
  bool debug = parser.isSet(dbgOption);
  int port = parser.value(portOption).toInt();

	
  EVProServer* server = new EVProServer(port, debug);
  server->Run();
  QObject::connect(server, &EVProServer::closed, &app, &QCoreApplication::quit);


  return app.exec();
}