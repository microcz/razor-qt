#include "razornotification.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QVariantMap>
#include <QDebug>
#include <QStringList>
#include <QMessageBox>


namespace
{
    const QString g_scFreedesktopNotificationName = "org.freedesktop.Notifications";
    const QString g_scSignalName = "Notify";
}

class RazorNotificationPrivate
{
public:
    RazorNotificationPrivate():
        m_bus(QDBusConnection::sessionBus())
    {
    }

    QDBusConnection m_bus ;
    quint32 notify(const QString& appName,
                   const QString& appIcon,
                   const QString& summary,
                   const QString& body,
                   const QVariantMap& hints,
                   quint32 replace_id,
                   qint32 expire_timeout)
    {
        QDBusMessage m = QDBusMessage::createMethodCall(QString::fromLatin1("org.freedesktop.Notifications"),
                                                        QString::fromLatin1("/org/freedesktop/Notifications"),
                                                        QString::fromLatin1("org.freedesktop.Notifications"),
                                                        QString::fromLatin1("Notify"));

        quint32 ret=0;
        QList<QVariant> args;
        QStringList actions;

        args.append(appName); // app_name
        args.append( uint(0) ); // replaces_id
        args.append(appIcon); // app_icon
        args.append(summary); // summary
        args.append(body); // body
        args.append(actions); // body

        args.append(hints); // hints
        args.append(expire_timeout); // timeout
        m.setArguments(args);
        QDBusMessage resp= m_bus.call(m);
        QVariantList retArgs = resp.arguments();
        qDebug() << retArgs << "size=" << retArgs.size() << resp.errorMessage();
        if ( retArgs.size() != 1)
        {
            qWarning() << "Unexpected number of retunred values";
        }
        else
        {
            ret = retArgs.at(0).toUInt();
        }
        
        // failback if there is no daemon running
        if (m_bus.lastError().isValid() || !m.errorName().isEmpty())
        {
            qWarning() << "Notification message cannot be delivered. Using failback QMessageBox";
            qWarning() << "    " << m_bus.lastError().message() << m.errorName();
            QMessageBox::information(0, QObject::tr("Razor Notification Failback"), summary + "<p>" + body);
        }

        return ret ;
    }

    void closeNotification( quint32 id )
    {
        QDBusMessage m = QDBusMessage::createMethodCall(QString::fromLatin1("org.freedesktop.Notifications"),
                                                        QString::fromLatin1("/org/freedesktop/Notifications"),
                                                        QString::fromLatin1("org.freedesktop.Notifications"),
                                                        QString::fromLatin1("CloseNotification"));
        QList<QVariant> args;
        args.append(id);
        m.setArguments(args);
        m_bus.call(m);
    }
};

RazorNotification::RazorNotification():
    d_ptr( new RazorNotificationPrivate)
{
}

quint32 RazorNotification::notify(const QString& appName,
                                  const QString& appIcon,
                                  const QString& summary,
                                  const QString& body,
                                  const QVariantMap& hints,
                                  quint32 replace_id,
                                  qint32 expire_timeout)
{
    RazorNotification n ;
    return n.d_func()->notify(appName, appIcon, summary, body, hints, replace_id, expire_timeout);
}

void RazorNotification::closeNotification(quint32 id)
{
    RazorNotification n ;
    n.d_func()->closeNotification(id);
}
