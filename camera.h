#ifndef CAMERA_H
#define CAMERA_H
#include <QtCore>
#include <QQuickItem>
class Camera:public QQuickItem
{
    Q_OBJECT
public:
    Camera(QQuickItem *parent=nullptr);
    ~Camera();
};



#endif // CAMERA_H
