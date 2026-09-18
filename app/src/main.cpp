#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStandardPaths>

#include "AppBackend.hpp"

int main(int argc, char* argv[]) {
  // Sin fijar esto, Qt Quick Controls elige un estilo por plataforma
  // (Material en Android, Fusion/Basic en escritorio) -- fijar "Basic" en
  // todas partes hace que el movil se comporte igual que el escritorio
  // mientras se desarrolla la interfaz (mismo criterio que crusader).
  QQuickStyle::setStyle("Basic");

  QGuiApplication app(argc, argv);
  QGuiApplication::setOrganizationName("LevelUp");
  QGuiApplication::setApplicationName("level_up");
  app.setWindowIcon(QIcon(QStringLiteral(":/qt/qml/LevelUp/qml/assets/level_up_icon.png")));

  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(dataDir);
  AppBackend backend((dataDir + "/level_up.sqlite3").toStdString());

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("backend", &backend);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("LevelUp", "Main");

  return app.exec();
}
