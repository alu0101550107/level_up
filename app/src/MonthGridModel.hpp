#pragma once

#include <QAbstractListModel>
#include <QDate>
#include <vector>

#include "levelup/Event.hpp"

// 42 celdas (6 semanas x 7 dias, lunes primero) para pintar la vista de
// mes como un GridView. Las celdas de meses adyacentes (relleno al
// principio/final) solo llevan isCurrentMonth=false -- no se decoran con
// hasEvents/allDone en este pase, simplificacion deliberada.
class MonthGridModel : public QAbstractListModel {
  Q_OBJECT
 public:
  enum Role {
    DateRole = Qt::UserRole + 1,
    DayNumberRole,
    IsCurrentMonthRole,
    HasEventsRole,
    AllDoneRole,
    IsTodayRole,
  };

  using QAbstractListModel::QAbstractListModel;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setMonth(int year, int month, std::vector<levelup::DaySummary> summaries);

 private:
  struct Cell {
    QDate date;
    bool isCurrentMonth = false;
    bool hasEvents = false;
    bool allDone = false;
  };
  std::vector<Cell> cells_;
};
