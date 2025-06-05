#include "AudioPlayer.h"
#include "StationDBModel.h"
#include <QtGlobal>
#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

// #include "app.h"
#include "version-transistor.h"
#include <KAboutData>
#include <KIconTheme>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "AudioPlayer.h"
#include "ColorSchemer.h"
#include "StationSearchModel.h"
#include "transistorconfig.h"

using namespace Qt::Literals::StringLiterals;

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char *argv[])
{
    KIconTheme::initTheme();
    if (TransistorConfig::self() && TransistorConfig::self()->interfaceMode() != 2) {
        if (TransistorConfig::self()->interfaceMode() == 0) {
            qunsetenv("QT_QUICK_CONTROLS_MOBILE");
        } else if (TransistorConfig::self()->interfaceMode() == 1) {
            qputenv("QT_QUICK_CONTROLS_MOBILE", "1");
        }
    }

    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << QStringLiteral(":icons"));

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    QApplication app(argc, argv);

    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(u"org.kde.desktop"_s);
    }
#endif

#ifdef Q_OS_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    QApplication::setStyle(QStringLiteral("breeze"));
    auto font = app.font();
    font.setPointSize(10);
    app.setFont(font);
#endif

    KLocalizedString::setApplicationDomain("transistor");
    QCoreApplication::setOrganizationName(u"KDE"_s);

    KAboutData aboutData(
        // The program name used internally.
        u"transistor"_s,
        // A displayable program name string.
        i18nc("@title", "Transistor"),
        // The program version string.
        QStringLiteral(TRANSISTOR_VERSION_STRING),
        // Short description of what the app does.
        i18n("Internet radio player"),
        // The license this code is released under.
        KAboutLicense::GPL,
        // Copyright Statement.
        i18n("(c) 2024"));
    aboutData.addAuthor(i18nc("@info:credit", "Yuri Saurov"), i18nc("@info:credit", "Maintainer"), u"dr@i-glu4it.ru"_s, u"https://i-glu4it.ru"_s);
    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    KAboutData::setApplicationData(aboutData);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"org.kde.transistor"_s));

    QQmlApplicationEngine engine;

    qmlRegisterSingletonInstance("org.kde.transistor.settings", 1, 0, "TransistorConfig", TransistorConfig::self());

    if (!TransistorConfig::self()->colorScheme().isEmpty()) {
        ColorSchemer::instance().apply(TransistorConfig::self()->colorScheme());
    }

    AudioPlayer *audioPlayer = new AudioPlayer(&app);
    qmlRegisterSingletonInstance("org.kde.transistor", 1, 0, "AudioPlayer", audioPlayer);

    StationSearchModel *stationSearchModel = new StationSearchModel(&app);
    qmlRegisterSingletonInstance("org.kde.transistor", 1, 0, "StationSearchModel", stationSearchModel);

    StationDBModel *stationDBModel = new StationDBModel(&app);
    qmlRegisterSingletonInstance("org.kde.transistor", 1, 0, "StationDBModel", stationDBModel);

    StationManager *stationManager = StationManager::instance();
    qmlRegisterSingletonInstance("org.kde.transistor", 1, 0, "StationManager", stationManager);
    
    QObject::connect(&app, &QCoreApplication::aboutToQuit, TransistorConfig::self(), &TransistorConfig::save);
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.loadFromModule("org.kde.transistor", u"Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
