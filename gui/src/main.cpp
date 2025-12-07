#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "logofile.h"
#include "logolistmodel.h"
#include "thumbnailprovider.h"
#include "appcontext.h"
#include "logolayer.h"

#ifndef BOOTMOD_VERSION
#define BOOTMOD_VERSION "1.6.0"
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
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
    
    QQmlApplicationEngine engine;
    
    // Expose version to QML
    engine.rootContext()->setContextProperty("appVersion", QString(BOOTMOD_VERSION));
    
    // Register image provider for thumbnails
    engine.addImageProvider("thumbnail", thumbnailProvider);
    
    // Load main QML
    const QUrl url(QStringLiteral("qrc:/BootMod/qml/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
}
