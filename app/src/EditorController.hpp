#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

namespace levelup {
class EventRepository;
}
class EventBus;

// API de escritura pura -- sin modelos propios. Crear/editar eventos pasa
// por AddEventDialog.qml (un unico dialogo para ambos casos), y las listas
// de eventos ya las muestran DayController/MonthController.
class EditorController : public QObject {
  Q_OBJECT

 public:
  EditorController(levelup::EventRepository& repo, EventBus& bus, QObject* parent = nullptr);

  Q_INVOKABLE void addWeeklyEvent(int dayOfWeek, const QString& title, const QString& time,
                                   const QString& endTime);
  Q_INVOKABLE void addOneOffEvent(const QString& date, const QString& title, const QString& time,
                                   const QString& endTime);
  Q_INVOKABLE void updateEvent(qlonglong id, const QString& title, const QString& time,
                                const QString& endTime);
  Q_INVOKABLE void deleteEvent(qlonglong id);

  // { "title": string, "time": string, "endTime": string } (vacios si no
  // tienen) -- para precargar AddEventDialog en modo edicion. Mapa vacio
  // si no existe.
  Q_INVOKABLE QVariantMap getEvent(qlonglong id) const;

 private:
  levelup::EventRepository& repo_;
  EventBus& bus_;
};
