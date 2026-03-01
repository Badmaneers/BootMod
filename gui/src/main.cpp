#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <QSettings>
#include <QQuickWindow>

static void applyWindowsDarkTitleBar(QObject *rootObject) {
    auto *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;
    QSettings reg(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    BOOL dark = (reg.value("AppsUseLightTheme", 1).toInt() == 0) ? TRUE : FALSE;
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    // 20 = DWMWA_USE_IMMERSIVE_DARK_MODE (Win10 20H1+ / Win11)
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
    // 19 = fallback for older Win10 builds
    DwmSetWindowAttribute(hwnd, 19, &dark, sizeof(dark));
}
#endif

#include "logofile.h"
#include "logolistmodel.h"
#include "thumbnailprovider.h"
#include "appcontext.h"
#include "logolayer.h"
#include "commandmanager.h"
#include "bitmapeditorwrapper.h"

#ifndef BOOTMOD_VERSION
#define BOOTMOD_VERSION "1.6.0"
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // App icon — SVG is in the Qt resource system; works for taskbar + window chrome
    app.setWindowIcon(QIcon(":/BootMod/res/bootmod_logo.svg"));

    app.setOrganizationName("BootMod");
    app.setApplicationName("BootMod");
    app.setApplicationVersion(BOOTMOD_VERSION);
    
    // Initialize app context
    ThumbnailProvider *thumbnailProvider = new ThumbnailProvider();
    AppContext::instance()->setThumbnailProvider(thumbnailProvider);
    
    // Register types
    qmlRegisterType<LogoFile>("BootMod", 1, 0, "LogoFile");
    qmlRegisterType<LogoListModel>("BootMod", 1, 0, "LogoListModel");
    qmlRegisterType<LogoLayer>("BootMod", 1, 0, "LogoLayer");
    qmlRegisterType<LayerManager>("BootMod", 1, 0, "LayerManager");
    qmlRegisterType<CommandManager>("BootMod", 1, 0, "CommandManager");
    qmlRegisterType<BitmapEditorWrapper>("BootMod", 1, 0, "BitmapEditorWrapper");
    
    QQmlApplicationEngine engine;
    
    // Expose version to QML
    engine.rootContext()->setContextProperty("appVersion", QString(BOOTMOD_VERSION));
    
    // Register image provider for thumbnails
    engine.addImageProvider("thumbnail", thumbnailProvider);
    
    // Load main QML
    const QUrl url(QStringLiteral("qrc:/BootMod/qml/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
            return;
        }
        if (obj && url == objUrl) {
#ifdef Q_OS_WIN
            applyWindowsDarkTitleBar(obj);
#endif
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
}
