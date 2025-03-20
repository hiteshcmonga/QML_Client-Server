#include "USBHelper.h"
#include <QProcess>
#include <QSysInfo>
#include <QDebug>

QString USBHelper::listDevices() {
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);

    if (QSysInfo::productType() == "windows") {
        // Improved PowerShell command
        QString command =
            "Get-PnpDevice -PresentOnly | "
            "Where-Object { $_.InstanceId -match 'USB' } | "
            "Select-Object FriendlyName, Manufacturer, DeviceID | "
            "Format-Table -AutoSize | "
            "Out-String -Width 4096";

        proc.start("powershell", QStringList() << "-Command" << command);
    } else {
        proc.start("lsusb");
    }

    if (!proc.waitForFinished(5000)) { // 5-second timeout
        qWarning() << "USB detection timed out";
        return "Error: USB detection timeout";
    }

    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    qDebug() << "Raw USB output:" << output; // Debug output

    return output.trimmed();
}