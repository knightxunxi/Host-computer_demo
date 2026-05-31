#include "services/AlarmService.h"

#include "storage/AlarmRepository.h"
#include "storage/OperationLogRepository.h"

namespace upkun::services {

AlarmService::AlarmService(upkun::storage::AlarmRepository* alarmRepository,
    upkun::storage::OperationLogRepository* operationLogRepository,
    QObject* parent)
    : QObject(parent)
    , m_alarmRepository(alarmRepository)
    , m_operationLogRepository(operationLogRepository)
{
}

void AlarmService::processSnapshot(const upkun::domain::DeviceSnapshot& snapshot)
{
    const int alarmCode = snapshot.currentAlarmCode;
    if (alarmCode != 0 && alarmCode != m_currentAlarmCode) {
        m_currentAlarmCode = alarmCode;
        if (m_alarmRepository != nullptr) {
            m_alarmRepository->openAlarm(alarmCode, alarmName(alarmCode), alarmStation(alarmCode), alarmLevel(alarmCode));
        }
        if (m_operationLogRepository != nullptr) {
            m_operationLogRepository->append(QStringLiteral("报警触发"), QString::number(alarmCode), QStringLiteral("成功"), alarmName(alarmCode));
        }
        emit recordsChanged();
        return;
    }

    if (alarmCode == 0 && m_currentAlarmCode != 0) {
        const int clearedCode = m_currentAlarmCode;
        m_currentAlarmCode = 0;
        if (m_alarmRepository != nullptr) {
            m_alarmRepository->clearOpenAlarm(clearedCode);
        }
        if (m_operationLogRepository != nullptr) {
            m_operationLogRepository->append(QStringLiteral("报警恢复"), QString::number(clearedCode), QStringLiteral("成功"), alarmName(clearedCode));
        }
        emit recordsChanged();
    }
}

void AlarmService::acknowledgeCurrentAlarm(const QString& userName)
{
    if (m_currentAlarmCode == 0 || m_alarmRepository == nullptr) {
        return;
    }

    m_alarmRepository->acknowledgeOpenAlarm(m_currentAlarmCode, userName);
    if (m_operationLogRepository != nullptr) {
        m_operationLogRepository->append(QStringLiteral("报警确认"), QString::number(m_currentAlarmCode), QStringLiteral("成功"), userName);
    }
    emit recordsChanged();
}

void AlarmService::acknowledgeCurrentAlarm(const upkun::domain::User& user)
{
    if (m_currentAlarmCode == 0 || m_alarmRepository == nullptr) {
        return;
    }

    m_alarmRepository->acknowledgeOpenAlarm(m_currentAlarmCode, user.displayName);
    if (m_operationLogRepository != nullptr) {
        m_operationLogRepository->append(user, QStringLiteral("报警确认"), QString::number(m_currentAlarmCode), QStringLiteral("成功"), user.displayName);
    }
    emit recordsChanged();
}

QVector<QStringList> AlarmService::recentAlarmRows(int limit) const
{
    return m_alarmRepository != nullptr ? m_alarmRepository->recentRows(limit) : QVector<QStringList> {};
}

QVector<QStringList> AlarmService::alarmRows(const QString& state, const QString& level, const QString& station, const QString& keyword, int limit) const
{
    if (m_alarmRepository == nullptr) {
        return {};
    }

    upkun::storage::AlarmQueryFilter filter;
    filter.state = state;
    filter.level = level;
    filter.station = station;
    filter.keyword = keyword;
    filter.limit = limit;
    return m_alarmRepository->queryRows(filter);
}

QVector<QStringList> AlarmService::recentOperationRows(int limit) const
{
    return m_operationLogRepository != nullptr ? m_operationLogRepository->recentRows(limit) : QVector<QStringList> {};
}

QString AlarmService::alarmName(int code) const
{
    switch (code) {
    case 1001:
        return QStringLiteral("急停触发");
    case 1002:
        return QStringLiteral("安全门打开");
    case 1003:
        return QStringLiteral("气压不足");
    case 2001:
        return QStringLiteral("上料缺瓶");
    case 2002:
        return QStringLiteral("上料堵瓶");
    case 3001:
        return QStringLiteral("输送卡瓶");
    case 4001:
        return QStringLiteral("料液不足");
    case 4002:
        return QStringLiteral("灌装超时");
    case 5001:
        return QStringLiteral("缺盖");
    case 5002:
        return QStringLiteral("旋盖失败");
    case 6001:
        return QStringLiteral("检测设备异常");
    case 6002:
        return QStringLiteral("产品检测不合格");
    case 7001:
        return QStringLiteral("缺标签");
    case 7002:
        return QStringLiteral("喷码失败");
    case 8001:
        return QStringLiteral("剔除失败");
    case 9001:
        return QStringLiteral("下料满料");
    default:
        return QStringLiteral("未知报警");
    }
}

QString AlarmService::alarmStation(int code) const
{
    if (code >= 1000 && code < 2000) {
        return QStringLiteral("公共系统");
    }
    if (code >= 2000 && code < 3000) {
        return QStringLiteral("上料");
    }
    if (code >= 3000 && code < 4000) {
        return QStringLiteral("输送定位");
    }
    if (code >= 4000 && code < 5000) {
        return QStringLiteral("灌装");
    }
    if (code >= 5000 && code < 6000) {
        return QStringLiteral("旋盖");
    }
    if (code >= 6000 && code < 7000) {
        return QStringLiteral("检测");
    }
    if (code >= 7000 && code < 8000) {
        return QStringLiteral("贴标/喷码");
    }
    if (code >= 8000 && code < 9000) {
        return QStringLiteral("分拣剔除");
    }
    if (code >= 9000 && code < 10000) {
        return QStringLiteral("下料");
    }
    return QStringLiteral("未知");
}

QString AlarmService::alarmLevel(int code) const
{
    if (code >= 1000 && code < 2000) {
        return QStringLiteral("Critical");
    }
    return QStringLiteral("Warning");
}

} // namespace upkun::services
