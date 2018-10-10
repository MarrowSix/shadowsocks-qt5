#include "editdialog.h"
#include "ui_editdialog.h"
#include "ssvalidator.h"
#include "ip4validator.h"
#include "portvalidator.h"
#include <QFileDialog>

EditDialog::EditDialog(Connection *_connection, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDialog),
    connection(_connection)
{
    ui->setupUi(this);

    /* initialisation and validator setup */
    static const QStringList supportedMethodList =
            SSValidator::supportedMethodList();
    static QStringList supportCryptMethod = {
        "aes", "aes-128", "aes-192", "salsa20", "blowfish", "twofish",
        "cast5", "3des", "tea","xtea", "xor", "sm4", "none"
    };
    static QStringList supportMode = {
        "fast", "fast3", "fast2", "normal", "manual"
    };
    ui->encryptComboBox->addItems(supportedMethodList);
    IP4Validator *addrValidator = new IP4Validator(this);
    PortValidator *portValidator = new PortValidator(this);
    ui->serverPortEdit->setValidator(portValidator);
    ui->localPortEdit->setValidator(portValidator);
    //Maybe we shouldn't validate local address using IPv4 format?
    ui->localAddrEdit->setValidator(addrValidator);

    ui->nameEdit->setText(connection->profile.name);
    ui->serverAddrEdit->setText(connection->profile.serverAddress);
    ui->serverPortEdit->setText(QString::number(connection->profile.serverPort));
    ui->localAddrEdit->setText(connection->profile.localAddress);
    ui->localPortEdit->setText(QString::number(connection->profile.localPort));
    ui->httpRadioButton->setChecked(connection->profile.httpMode);
    ui->pwdEdit->setText(connection->profile.password);
    ui->encryptComboBox->setCurrentText(connection->profile.method.toUpper());
    ui->timeoutSpinBox->setValue(connection->profile.timeout);
    ui->resetDateEdit->setDate(connection->profile.nextResetDate);
    ui->resetDateEdit->setMinimumDate(QDate::currentDate());
    ui->autoStartCheckBox->setChecked(connection->profile.autoStart);

    ui->kcpclientLineEdit->setText(connection->profile.pathOfKcptunClient);
    ui->remoteaddrLineEdit->setText(connection->profile.remoteaddr);
    ui->keyLineEdit->setText(connection->profile.key);
    ui->cryptComboBox->addItems(supportCryptMethod);
    ui->cryptComboBox->setCurrentText(connection->profile.crypt);
    ui->modeComboBox->addItems(supportMode);
    ui->modeComboBox->setCurrentText(connection->profile.mode);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &EditDialog::save);
    connect(ui->openKcptunClient, &QPushButton::clicked, this, &EditDialog::openKcptunClient);
    this->adjustSize();
}

EditDialog::~EditDialog()
{
    delete ui;
}

void EditDialog::save()
{
    connection->profile.name = ui->nameEdit->text();
    connection->profile.serverAddress = ui->serverAddrEdit->text().trimmed();
    connection->profile.serverPort = ui->serverPortEdit->text().toUShort();
    connection->profile.localAddress = ui->localAddrEdit->text();
    connection->profile.localPort = ui->localPortEdit->text().toUShort();
    connection->profile.httpMode = ui->httpRadioButton->isChecked();
    connection->profile.password = ui->pwdEdit->text();
    connection->profile.method = ui->encryptComboBox->currentText();
    connection->profile.timeout = ui->timeoutSpinBox->value();
    connection->profile.nextResetDate = ui->resetDateEdit->date();
    connection->profile.autoStart = ui->autoStartCheckBox->isChecked();
    connection->profile.pathOfKcptunClient = ui->kcpclientLineEdit->text();
    connection->profile.remoteaddr = ui->remoteaddrLineEdit->text();
    connection->profile.localaddr = QString(":").append(QString::number(connection->profile.serverPort));
    connection->profile.key = ui->keyLineEdit->text().trimmed();
    connection->profile.crypt = ui->cryptComboBox->currentText();
    connection->profile.mode = ui->modeComboBox->currentText();

    this->accept();
}

void EditDialog::openKcptunClient()
{
    QString file = QFileDialog::getOpenFileName(
                   this,
                   tr("Import Kcptun Client Path"),
                   QString(),
                   "");
    if (!file.isNull()) {
        ui->kcpclientLineEdit->setText(file);
    }
}
