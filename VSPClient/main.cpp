#include <QApplication>
#include <QLocale>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QTranslator>
#include <vscmainwindow.h>

class ApplicationStyle: public QProxyStyle
{
public:
    int styleHint(
       StyleHint hint,
       const QStyleOption* option = nullptr, //
       const QWidget* widget = nullptr,      //
       QStyleHintReturn* returnData = nullptr) const override
    {
        switch (hint) {
            case QStyle::SH_ComboBox_Popup: {
                return 0;
            }
            case QStyle::SH_MessageBox_CenterButtons: {
                return 0;
            }
            case QStyle::SH_FocusFrame_AboveWidget: {
                return 1;
            }
            default: {
                break;
            }
        }
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

#if defined(VSPCLIENT_LIBRARY)
extern "C" bool qt_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    QApplication::setAttribute(Qt::AA_NativeWindows, true);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
    QApplication::setAttribute(Qt::AA_Use96Dpi, true);

    QApplication a(argc, argv);
    QApplication::setOrganizationName("EoF Software Labs");
    QApplication::setApplicationDisplayName(QStringLiteral("Virtual Serial Port Controller"));
    QApplication::setApplicationName("VSP Controller");
    QApplication::setApplicationVersion("1.6.40");
    QApplication::setDesktopSettingsAware(true);
    QApplication::setQuitOnLastWindowClosed(true);

    /* Override commandline style with our fixed GUI style type */
    /* macintosh, Windows, Fusion */
    QString styleName;

#if defined(Q_OS_UNIX)
    styleName = "Fusion";
#else
    styleName = "Windows";
#endif

    /* configure custom GUI style hinter */
    QStyle* style;
    if ((style = QStyleFactory::create(styleName))) {
        ApplicationStyle* myStyle = new ApplicationStyle();
        myStyle->setBaseStyle(style);
        a.setStyle(myStyle);
    }

    a.setStyleSheet(
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
       "*{font-size:16pt;}"
#endif
       "QPushButton {border-radius:5px;"
       "border-style:solid;"
       "border-width:1px;"
       "border-color:rgb(76,76,76);"
       "min-height:32px;min-width:130px;padding:4px,4px,4px,4px;}"
       "QPushButton::focus {border-color:white;}"
       "QPushButton::default {border-color:rgb(252,115,9);}"
       "QPushButton::pressed {border-color:rgb(252,115,9);}"
       "QPushButton::hover {border-color:rgb(252,115,9);}"
       "QPushButton::disabled {border-color:rgb(46,46,46);}"
#if defined(Q_OS_UNIX)
       "QComboBox {color: white;}"
#endif
    );

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
