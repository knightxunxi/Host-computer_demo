#pragma once

#include <QWidget>

namespace upkun::ui {

class SimulatorPage final : public QWidget {
    Q_OBJECT

public:
    explicit SimulatorPage(QWidget* parent = nullptr);
};

} // namespace upkun::ui
