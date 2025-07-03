#include "widgets/mainwindow.h"

#include <QApplication>
#include <QDir>

#include <qca-qt6/QtCrypto>

#include "constants.h"

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication a(argc, argv);

    QDir basePath(QCoreApplication::applicationDirPath());
    qDebug() << "The application's working directory: " << QDir::currentPath() << Qt::endl;

    QCoreApplication::addLibraryPath(basePath.absoluteFilePath("plugins"));
    qDebug() << "Application plugins library paths: " << QCoreApplication::libraryPaths().join(";") << Qt::endl;

    QCA::Initializer init;
    QStringList paths = QCA::pluginPaths();
    Q_FOREACH (const QString& path, paths)
        qDebug() << "QCA plugins path: " << QDir::toNativeSeparators(path) << Qt::endl;
    QCA::ProviderList list = QCA::providers();
    for (int n = 0; n < list.count(); ++n)
    {
        QString str = QCA::pluginDiagnosticText();
        qDebug() << "Available Plugin Diagnostic: " << str << Qt::endl;
        QCA::clearPluginDiagnosticText();

        qDebug() << "Available Providers: " << list[n]->name() << Qt::endl;

        QString credit = list[n]->credit();
        qDebug() << "Available Providers Credit: " << credit << Qt::endl;
    }
    QStringList features = QCA::supportedFeatures();
    Q_FOREACH (const QString& feature, features)
        qDebug() << "QCA feature: " << feature << Qt::endl;

    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);

    MainWindow mainwindow;
    emit mainwindow.loadClusters();
    mainwindow.show();

    return a.exec();
}
