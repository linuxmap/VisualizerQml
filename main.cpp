#include <QGuiApplication>
//#include <QWidget> 
#include <QtQuick/QQuickView>
#include <QQmlContext>
#include <QtQuickControls2/QQuickStyle>

//#include <QtDeclarative>

#define __MAIN__
#include "common.h"
#include "displayitem.h"
#include "uvccapture.h"
#include "painteditem.hpp"
#include "inputitem.hpp"
#include "uvcuacinfo.h"
#include "uvccontroler.h"
#if defined( _DEBUG)||defined(__DEBUG)
#ifndef QSG_INFO
#define QSG_INFO  1
#endif
#endif
int main(int argc, char **argv)
{
    g_programTimer.start();
    QGuiApplication app(argc, argv);
	QString workingDirectory= QDir::currentPath();
	//QApplication app(argc, argv);
	//app.setAttribute(Qt::AA_EnableHighDpiScaling);
	app.setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, true);


	qmlRegisterType<DisplayItem>("ophelia.qml.controls", 1, 0, "VideoDisplay");
	qmlRegisterType<PaintedItem>("ophelia.qml.controls", 1, 0, "AnotationLayer");
	qmlRegisterType<InputItem>("ophelia.qml.controls", 1, 0, "InputItem");
	qmlRegisterType<UvcUacInfo>("ophelia.qml.controls", 1, 0, "UvcUacInfo");
	qmlRegisterType<UvcCapture>("ophelia.qml.controls", 1, 0, "UvcCamera");
	qmlRegisterUncreatableType<UvcControler>("ophelia.qml.controls", 1, 0, "UvcControler", QObject::trUtf8("UvcControler is provided by UvcCamera"));
	qmlRegisterUncreatableType<UvcGrabber>("ophelia.qml.controls", 1, 0, "UvcImageCapture", QObject::trUtf8("UvcImageCapture is provided by UvcCamera"));
	qmlRegisterType<Ffplay>("ophelia.qml.controls", 1, 0, "FFMedia");
	qmlRegisterInterface<IFrameRenderer>("IFrameRenderer");

	//qmlRegisterInterface<IFrameRenderer>("ISampleGrabberCB");
	QQuickStyle::setStyle("Material");
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///visualizer.qml"));
	view.rootContext()->setContextProperty("window", &view);
	view.rootContext()->setContextProperty("workingdirectory", workingDirectory);
	view.rootContext()->setBaseUrl(QUrl(QString("file:///"+ workingDirectory)));
	QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));
    view.show();
    return app.exec();
}
