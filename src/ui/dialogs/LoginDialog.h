#pragma once

#include "domain/User.h"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QVector>

namespace upkun::ui {

class LoginDialog final : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(const QVector<upkun::domain::User>& users, QWidget* parent = nullptr);

    QString loginName() const;
    QString password() const;

private:
    QComboBox* m_userCombo = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
};

} // namespace upkun::ui
