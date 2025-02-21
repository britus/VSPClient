#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <vscmainwindow.h>

int main(int argc, char* argv[])
{
    QApplication::setAttribute(Qt::AA_NativeWindows, true);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
    QApplication::setAttribute(Qt::AA_Use96Dpi);

    QApplication a(argc, argv);
    QApplication::setOrganizationName("EoF Software Labs");
    QApplication::setApplicationDisplayName(QStringLiteral("Virtual Serial Port Controller"));
    QApplication::setApplicationName("VSP Controller");
    QApplication::setApplicationVersion("1.4.1");
    QApplication::setDesktopSettingsAware(true);
    QApplication::setQuitOnLastWindowClosed(true);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString& locale : uiLanguages) {
        const QString baseName = "vspui_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    VSCMainWindow w;
    w.show();

    return a.exec();
}
