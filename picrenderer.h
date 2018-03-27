#ifndef PICRENDERER_H
#define PICRENDERER_H

#include <QtGui/qvector3d.h>
#include <QtGui/qmatrix4x4.h>
#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglfunctions.h>

#include <QTime>
#include <QVector>
#include <QtOpenGL/QtOpenGL>
#include <QQuickFramebufferObject>

#include "common.h"
#if VIDEO_RENDER_QSG_FBO

class PicRenderer : public QOpenGLFunctions
{
public:
    PicRenderer();
    ~PicRenderer();
    void render();
    void initialize();
    void displayRawData(VideoFrame *frame);
	bool isNeedUpdate(void);
	void clearUpdateRequest();
protected:
private:
    void paintGL(void);
    bool videoResolChanged(void);

private:
	bool m_bNeedUpdate;
    enum VAO_IDs { Rectangle, Texture, VaoNum };
    enum VBO_IDs { RectBuffer, TexBuffer, VboNum };
    enum Attrib_IDs { RectAttrib=3, TexAttrib=4,AttribNum=2 };

    qreal   m_fAngle;
    qreal   m_fScale;

    void paintQtLogo();
    void createGeometry();
    void quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4);
    void extrude(qreal x1, qreal y1, qreal x2, qreal y2);

    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QOpenGLShaderProgram program1;
    int vertexAttr1;
    int normalAttr1;
    int matrixUniform1;



    GLuint textureUniformY; //y纹理数据位置
    GLuint textureUniformU; //u纹理数据位置
    GLuint textureUniformV; //v纹理数据位置
    GLuint id_y; //y纹理对象ID
    GLuint id_u; //u纹理对象ID
    GLuint id_v; //v纹理对象ID
    QOpenGLTexture* m_pTextureY;  //y纹理对象
    QOpenGLTexture* m_pTextureU;  //u纹理对象
    QOpenGLTexture* m_pTextureV;  //v纹理对象
    //QOpenGLShader *m_pVSHader;  //顶点着色器程序对象
    //QOpenGLShader *m_pFSHader;  //片段着色器对象
    QOpenGLShaderProgram *m_pYuv422pShaderProgram; //着色器程序容器
    QOpenGLShaderProgram *m_pYuy2ShaderProgram; //着色器程序容器
    VideoFrame *m_pVideoFrame;
    //QMap<QString, QOpenGLShaderProgram*>  *m_pVShaderProgramMap;
    QMap<QString, QOpenGLShaderProgram*>  *m_pShaderProgramMap;
    QOpenGLShaderProgram* m_pCurShaderProgram;
    QMap<QString, QOpenGLShader*>  *m_pVShaderMap;
    QString m_currVShader;
    QMap<QString, QOpenGLShader*>  *m_pFShaderMap;
    QString m_currFShader;

    int m_curVideoW;
    int m_curVideoH;
    GLuint VAOs[VaoNum];
    GLuint VBOs[VboNum];
    GLuint Attribs[AttribNum];
};
#endif
#endif
