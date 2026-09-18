#pragma once

#include <QObject>

// Punto central de notificacion: cualquier mutacion de datos (crear/borrar
// un evento, marcar/desmarcar un check) emite `changed()`, y los tres
// controllers (Day/Month/Editor) escuchan para re-consultar. Sin esto, un
// evento creado en el Editor no aparaceria en Dia/Mes hasta reiniciar la
// app -- las tres pestañas comparten la misma base de datos pero no tienen
// otra forma de enterarse de los cambios de las demas.
class EventBus : public QObject {
  Q_OBJECT
 public:
  using QObject::QObject;

  void notify() { emit changed(); }

 signals:
  void changed();
};
