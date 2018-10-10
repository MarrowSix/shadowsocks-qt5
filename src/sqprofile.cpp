#include "sqprofile.h"

SQProfile::SQProfile()
{
    autoStart = false;
    debug = false;
    serverPort = 8388;
    localPort = 1080;
    name = QObject::tr("Unnamed Profile");
    localAddress = QString("127.0.0.1");
    method = QString("RC4-MD5");
    timeout = 600;
    latency = LATENCY_UNKNOWN;
    currentUsage = 0;
    totalUsage = 0;
    QDate currentDate = QDate::currentDate();
    nextResetDate = QDate(currentDate.year(), currentDate.month() + 1, 1);
    httpMode = false;
    pathOfKcptunClient = QObject::tr("");
    remoteaddr = "vps:29900";
    localaddr = ":12948";
    key = "it's a secrect";
    crypt = "aes";
    mode = "fast";
}

SQProfile::SQProfile(const QSS::Profile &profile) : SQProfile()
{
    name = QString::fromStdString(profile.name());
    localAddress = QString::fromStdString(profile.localAddress());
    localPort = profile.localPort();
    serverPort = profile.serverPort();
    serverAddress = QString::fromStdString(profile.serverAddress());
    method = QString::fromStdString(profile.method()).toUpper();
    password = QString::fromStdString(profile.password());
    timeout = profile.timeout();
    httpMode = profile.httpProxy();
    debug = profile.debug();
    pathOfKcptunClient = QObject::tr("");
    remoteaddr = "vps:29900";
    localaddr = QString(":").append(localPort);
    key = "it's a secrect";
    crypt = "aes";
    mode = "fast";
}

SQProfile::SQProfile(const QString &uri)
{
    *this = SQProfile(QSS::Profile::fromUri(uri.toStdString()));
}

QSS::Profile SQProfile::toProfile() const
{
    QSS::Profile qssprofile;
    qssprofile.setName(name.toStdString());
    qssprofile.setServerAddress(serverAddress.toStdString());
    qssprofile.setServerPort(serverPort);
    qssprofile.setLocalAddress(localAddress.toStdString());
    qssprofile.setLocalPort(localPort);
    qssprofile.setMethod(method.toLower().toStdString());
    qssprofile.setPassword(password.toStdString());
    qssprofile.setTimeout(timeout);
    qssprofile.setHttpProxy(httpMode);
    if (debug) {
        qssprofile.enableDebug();
    } else {
        qssprofile.disableDebug();
    }
    return qssprofile;
}

const QStringList& SQProfile::toArgument()
{
    kcpArgument << QString("-r " + remoteaddr)
                << QString("-l " + localaddr)
                << QString("-key " + key)
                << QString("-crypt " + crypt)
                << QString("-mode " + mode);
    return kcpArgument;
}

QDataStream& operator << (QDataStream &out, const SQProfile &p)
{
    out << p.autoStart << p.debug << p.serverPort << p.localPort << p.name << p.serverAddress << p.localAddress << p.method << p.password << p.timeout << p.latency << p.currentUsage << p.totalUsage << p.lastTime << p.nextResetDate << p.httpMode
        << p.pathOfKcptunClient << p.remoteaddr << p.localaddr << p.key << p.crypt << p.mode;
    return out;
}

QDataStream& operator >> (QDataStream &in, SQProfile &p)
{
    in >> p.autoStart >> p.debug >> p.serverPort >> p.localPort >> p.name >> p.serverAddress >> p.localAddress >> p.method >> p.password >> p.timeout >> p.latency >> p.currentUsage >> p.totalUsage >> p.lastTime >> p.nextResetDate >> p.httpMode
       >> p.pathOfKcptunClient >> p.remoteaddr >> p.localaddr >> p.key >> p.crypt >> p.mode;
    return in;
}
