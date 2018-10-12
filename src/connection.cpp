#include "connection.h"
#include "ssvalidator.h"
#include <QFile>
#include <QHostInfo>
#include <QHostAddress>

Connection::Connection(QObject *parent) :
    QObject(parent),
    running(false),
    kcpProcess(nullptr)
{}

Connection::Connection(const SQProfile &_profile, QObject *parent) :
    Connection(parent)
{
    profile = _profile;
}

Connection::Connection(QString uri, QObject *parent) :
    Connection(parent)
{
    profile = SQProfile(uri);
}

Connection::~Connection()
{
    stop();
    if (!kcpProcess)
        delete kcpProcess;
}

const SQProfile& Connection::getProfile() const
{
    return profile;
}

const QString& Connection::getName() const
{
    return profile.name;
}

QByteArray Connection::getURI() const
{
    std::string uri = profile.toProfile().toUriSip002();
    return QByteArray(uri.data(), uri.length());
}

bool Connection::isValid() const
{
    if (profile.serverAddress.isEmpty() || profile.localAddress.isEmpty() ||
        profile.timeout < 1 || !SSValidator::validateMethod(profile.method) ||
        !QFile::exists(profile.pathOfKcptunClient)) {
        return false;
    }
    else {
        return true;
    }
}

const bool &Connection::isRunning() const
{
    return running;
}

void Connection::latencyTest()
{
    QHostAddress serverAddr(profile.serverAddress);
    if (serverAddr.isNull()) {
        QHostInfo::lookupHost(profile.serverAddress, this, SLOT(onServerAddressLookedUp(QHostInfo)));
    } else {
        testAddressLatency(serverAddr);
    }
}

void Connection::start()
{
    profile.lastTime = QDateTime::currentDateTime();
    //perform a latency test if the latency is unknown
    if (profile.latency == SQProfile::LATENCY_UNKNOWN) {
        latencyTest();
    }

    controller = std::make_unique<QSS::Controller>(profile.toProfile(), true, false);
    connect(controller.get(), &QSS::Controller::runningStateChanged, [&](bool run){
        running = run;
        emit stateChanged(run);
    });
    connect(controller.get(), &QSS::Controller::tcpLatencyAvailable, this, &Connection::onLatencyAvailable);
    connect(controller.get(), &QSS::Controller::newBytesReceived, this, &Connection::onNewBytesTransmitted);
    connect(controller.get(), &QSS::Controller::newBytesSent, this, &Connection::onNewBytesTransmitted);

    if (!controller->start()) {
        emit startFailed();
    }

    kcpProcess = new QProcess(this);
    QStringList argument = profile.toArgument();

    connect(kcpProcess, &QProcess::errorOccurred, this, &Connection::processError);
    connect(kcpProcess, &QProcess::readyReadStandardOutput, this, &Connection::printStdoutLog);
    connect(kcpProcess, &QProcess::readyReadStandardError, this, &Connection::printStderrLog);
//    connect(kcpProcess, &QProcess::readyRead, this, &Connection::printStdoutLog);
//    kcpProcess->setProgram(profile.pathOfKcptunClient);
//    kcpProcess->setArguments(argument);
//    kcpProcess->start();
    kcpProcess->start(profile.pathOfKcptunClient, argument);
    kcpProcess->waitForStarted();

    qDebug() << kcpProcess->state();
    qDebug() << kcpProcess->arguments();
    if (kcpProcess->state() == QProcess::NotRunning) {
        emit startFailed();
    }
}

void Connection::stop()
{
    if (running) {
        controller.reset();
        kcpProcess->kill();
        delete kcpProcess;
    }
}

void Connection::processError(QProcess::ProcessError error)
{
    switch(error) {
        case QProcess::FailedToStart:
            qDebug() << "Kcptun Client FailedToStart";
            break;
        case QProcess::Crashed:
            qDebug() << "Kcptun Client Creashed";
            break;
        case QProcess::Timedout:
            qDebug() << "Kcptun Client Timeout";
            break;
        case QProcess::WriteError:
            qDebug() << "Kcptun Client WriteError";
            break;
        case QProcess::ReadError:
            qDebug() << "Kcptun Client ReadError";
            break;
        case QProcess::UnknownError:
            qDebug() << "Kcptun Client UnknowError";
            break;
        default:
            qDebug() << "Kcptun Client UnknowError";
            break;
    }
}

void Connection::printStdoutLog()
{
    QString result = kcpProcess->readAllStandardOutput();
    QStringList tempResult = result.split('\n');
    for (int i=0; i<tempResult.size(); i++) {
        qDebug() << tempResult.at(i);
    }
}

void Connection::printStderrLog()
{
    QString result = kcpProcess->readAllStandardError();
    QStringList tempResult = result.split('\n');
    for (int i=0; i<tempResult.size(); i++) {
        qDebug() << tempResult.at(i);
    }
}

void Connection::testAddressLatency(const QHostAddress &addr)
{
    QSS::AddressTester *addrTester = new QSS::AddressTester(addr, profile.serverPort, this);
    connect(addrTester, &QSS::AddressTester::connectivityTestFinished, this, &Connection::onConnectivityTestFinished, Qt::QueuedConnection);
    connect(addrTester, &QSS::AddressTester::lagTestFinished, this, &Connection::onLatencyAvailable, Qt::QueuedConnection);
    QSS::Profile qProfile = profile.toProfile();
    addrTester->startConnectivityTest(qProfile.method(), qProfile.password());
}

void Connection::onNewBytesTransmitted(const quint64 &b)
{
    profile.currentUsage += b;
    profile.totalUsage += b;
    emit dataUsageChanged(profile.currentUsage, profile.totalUsage);
}

void Connection::onServerAddressLookedUp(const QHostInfo &host)
{
    if (host.error() == QHostInfo::NoError) {
        testAddressLatency(host.addresses().first());
    } else {
        onLatencyAvailable(SQProfile::LATENCY_ERROR);
    }
}

void Connection::onLatencyAvailable(const int latency)
{
    profile.latency = latency;
    emit latencyAvailable(latency);
}

void Connection::onConnectivityTestFinished(bool con)
{
    QSS::AddressTester* tester = qobject_cast<QSS::AddressTester*>(sender());
    if (!con) {
        disconnect(tester, &QSS::AddressTester::lagTestFinished, this, &Connection::onLatencyAvailable);
        this->onLatencyAvailable(SQProfile::LATENCY_ERROR);
        qWarning("Internet connectivity test failed. Please check the connection's profile and your firewall settings.");
    }
    tester->deleteLater();
}
