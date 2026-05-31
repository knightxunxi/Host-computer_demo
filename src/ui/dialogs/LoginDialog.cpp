#include "ui/dialogs/LoginDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace upkun::ui {

LoginDialog::LoginDialog(const QVector<upkun::domain::User>& users, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("切换用户"));
    setModal(true);
    setMinimumWidth(360);
    setStyleSheet(QStringLiteral(
        "QDialog { background: #ffffff; color: #000000; }"
        "QLabel { color: #000000; }"
        "QComboBox, QLineEdit { background: #ffffff; color: #000000; border: 1px solid #a0a0a0; min-height: 30px; padding: 2px 6px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 30px; padding: 0 14px; }"
        "QPushButton:hover { background: #eeeeee; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 16, 18, 14);
    rootLayout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("选择用户并输入密码"), this);
    title->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);

    m_userCombo = new QComboBox(this);
    for (const auto& user : users) {
        m_userCombo->addItem(QStringLiteral("%1（%2，%3）")
                .arg(user.displayName, user.loginName, upkun::domain::userRoleText(user.role)),
            user.loginName);
    }

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));

    form->addRow(QStringLiteral("用户"), m_userCombo);
    form->addRow(QStringLiteral("密码"), m_passwordEdit);
    rootLayout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("登录"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    rootLayout->addWidget(buttons);
}

QString LoginDialog::loginName() const
{
    return m_userCombo != nullptr ? m_userCombo->currentData().toString() : QString {};
}

QString LoginDialog::password() const
{
    return m_passwordEdit != nullptr ? m_passwordEdit->text() : QString {};
}

} // namespace upkun::ui
