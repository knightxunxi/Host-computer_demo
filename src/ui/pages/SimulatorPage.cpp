#include "ui/pages/SimulatorPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace upkun::ui {

SimulatorPage::SimulatorPage(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(18);

    auto* title = new QLabel(QStringLiteral("模拟器"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #111827;"));
    rootLayout->addWidget(title);

    auto* hint = new QLabel(QStringLiteral("后续这里用于启动虚拟 PLC、设置合格率、触发缺瓶/堵瓶/缺盖/急停等模拟故障。"), this);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color: #4b5563;"));
    rootLayout->addWidget(hint);

    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("simPanel"));
    panel->setStyleSheet(QStringLiteral(
        "#simPanel { background: #ffffff; border: 1px solid #d8dee6; border-radius: 6px; }"
        "QPushButton { min-height: 34px; padding: 0 16px; }"));

    auto* grid = new QGridLayout(panel);
    grid->setContentsMargins(16, 16, 16, 16);
    grid->setSpacing(12);
    grid->addWidget(new QPushButton(QStringLiteral("启动模拟器"), panel), 0, 0);
    grid->addWidget(new QPushButton(QStringLiteral("停止模拟器"), panel), 0, 1);
    grid->addWidget(new QPushButton(QStringLiteral("触发缺瓶"), panel), 1, 0);
    grid->addWidget(new QPushButton(QStringLiteral("触发缺盖"), panel), 1, 1);
    grid->addWidget(new QPushButton(QStringLiteral("触发缺标签"), panel), 2, 0);
    grid->addWidget(new QPushButton(QStringLiteral("清除模拟故障"), panel), 2, 1);

    rootLayout->addWidget(panel);
    rootLayout->addStretch(1);
}

} // namespace upkun::ui
