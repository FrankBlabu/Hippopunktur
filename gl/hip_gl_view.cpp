/*
 * hip_gl_view.cpp - View displaying a GL scene
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGLView.h"
#include "HIPGLModel.h"
#include "ui_hip_gl_view.h"

#include "core/HIPException.h"
#include "core/HIPTools.h"

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#include <QWheelEvent>

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // DEBUG
    //#**********************************************************************

#if 0
    namespace {

      void GLWidget::createGeometry()
      {
        vertices.clear();
        normals.clear();

        qreal x1 = +0.06f;
        qreal y1 = -0.14f;
        qreal x2 = +0.14f;
        qreal y2 = -0.06f;
        qreal x3 = +0.08f;
        qreal y3 = +0.00f;
        qreal x4 = +0.30f;
        qreal y4 = +0.22f;

        quad(x1, y1, x2, y2, y2, x2, y1, x1);
        quad(x3, y3, x4, y4, y4, x4, y3, x3);

        extrude(x1, y1, x2, y2);
        extrude(x2, y2, y2, x2);
        extrude(y2, x2, y1, x1);
        extrude(y1, x1, x1, y1);
        extrude(x3, y3, x4, y4);
        extrude(x4, y4, y4, x4);
        extrude(y4, x4, y3, x3);

        const qreal Pi = 3.14159f;
        const int NumSectors = 100;

        for (int i = 0; i < NumSectors; ++i) {
            qreal angle1 = (i * 2 * Pi) / NumSectors;
            qreal x5 = 0.30 * sin(angle1);
            qreal y5 = 0.30 * cos(angle1);
            qreal x6 = 0.20 * sin(angle1);
            qreal y6 = 0.20 * cos(angle1);

            qreal angle2 = ((i + 1) * 2 * Pi) / NumSectors;
            qreal x7 = 0.20 * sin(angle2);
            qreal y7 = 0.20 * cos(angle2);
            qreal x8 = 0.30 * sin(angle2);
            qreal y8 = 0.30 * cos(angle2);

            quad(x5, y5, x6, y6, x7, y7, x8, y8);

            extrude(x6, y6, x7, y7);
            extrude(x8, y8, x5, y5);
          }

        for (int i = 0;i < vertices.size();i++)
          vertices[i] *= 2.0f;

        qDebug () << "* Render data:";
        qDebug () << "  " << vertices.size () << " vertices";
        qDebug () << "  " << normals.size () << "  normals";

        foreach (const QVector3D& v, vertices)
          qDebug () << "  v=" << v;

        foreach (const QVector3D& n, normals)
          qDebug () << "  n=" << n;
      }

      void quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4)
      {
        vertices << QVector3D(x1, y1, -0.05f);
        vertices << QVector3D(x2, y2, -0.05f);
        vertices << QVector3D(x4, y4, -0.05f);

        vertices << QVector3D(x3, y3, -0.05f);
        vertices << QVector3D(x4, y4, -0.05f);
        vertices << QVector3D(x2, y2, -0.05f);

        QVector3D n = QVector3D::normal
            (QVector3D(x2 - x1, y2 - y1, 0.0f), QVector3D(x4 - x1, y4 - y1, 0.0f));

        normals << n;
        normals << n;
        normals << n;

        normals << n;
        normals << n;
        normals << n;

        vertices << QVector3D(x4, y4, 0.05f);
        vertices << QVector3D(x2, y2, 0.05f);
        vertices << QVector3D(x1, y1, 0.05f);

        vertices << QVector3D(x2, y2, 0.05f);
        vertices << QVector3D(x4, y4, 0.05f);
        vertices << QVector3D(x3, y3, 0.05f);

        n = QVector3D::normal
            (QVector3D(x2 - x4, y2 - y4, 0.0f), QVector3D(x1 - x4, y1 - y4, 0.0f));

        normals << n;
        normals << n;
        normals << n;

        normals << n;
        normals << n;
        normals << n;
      }

      void extrude(qreal x1, qreal y1, qreal x2, qreal y2)
      {
        vertices << QVector3D(x1, y1, +0.05f);
        vertices << QVector3D(x2, y2, +0.05f);
        vertices << QVector3D(x1, y1, -0.05f);

        vertices << QVector3D(x2, y2, -0.05f);
        vertices << QVector3D(x1, y1, -0.05f);
        vertices << QVector3D(x2, y2, +0.05f);

        QVector3D n = QVector3D::normal
            (QVector3D(x2 - x1, y2 - y1, 0.0f), QVector3D(0.0f, 0.0f, -0.1f));

        normals << n;
        normals << n;
        normals << n;

        normals << n;
        normals << n;
        normals << n;
      }

    }
#endif

    //#**********************************************************************
    // CLASS HIP::GL::Widget
    //#**********************************************************************

    /*
     * Widget displaying an open GL scene
     */
    class Widget : public QOpenGLWidget, protected QOpenGLFunctions
    {
    public:
      Widget (const QString& model_path, QWidget* parent);
      virtual ~Widget ();

      virtual void initializeGL ();
      virtual void resizeGL (int width, int height);
      virtual void paintGL ();

    protected:
      virtual void keyPressEvent (QKeyEvent* event);

      virtual void mousePressEvent (QMouseEvent* event);
      virtual void mouseMoveEvent (QMouseEvent* event);
      virtual void mouseReleaseEvent (QMouseEvent* event);
      virtual void wheelEvent (QWheelEvent* event);

    private:
      Model* _model;
      QOpenGLShaderProgram _shader;

      QMatrix4x4 _projection;
      QMatrix4x4 _mvp;

      QVector<QVector3D> _vertices;
      QVector<QVector3D> _normals;

      QPointF _last_pos;
    };


    /*! Constructor */
    Widget::Widget (const QString& model_path, QWidget* parent)
      : QOpenGLWidget (parent),
        _model      (new Model (model_path)),
        _shader     (),
        _projection (),
        _mvp        (),
        _vertices   (),
        _normals    (),
        _last_pos   (0, 0)
    {
      setFocusPolicy (Qt::WheelFocus);

      QSurfaceFormat format;
      format.setDepthBufferSize (24);
      format.setStencilBufferSize (8);
      setFormat (format);
    }

    /*! Destructor */
    Widget::~Widget ()
    {
      makeCurrent ();

      // Delete GL related structures

      doneCurrent ();

      delete _model;
    }

    /*
     * Initialize GL widget
     */
    void Widget::initializeGL ()
    {
      initializeOpenGLFunctions ();

      glClearColor (0.1f, 0.1f, 0.1f, 1.0f);
      glFrontFace (GL_CW);
      glEnable (GL_CULL_FACE);
      glEnable (GL_DEPTH_TEST);

      //
      // Init shaders
      //
      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Vertex, ":/gl/VertexShader.glsl"))
        throw Exception (tr ("Unable to initialize vertex shader."));

      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Fragment, ":/gl/FragmentShader.glsl"))
        throw Exception (tr ("Unable to initialize fragment shader."));

      if (!_shader.link ())
        throw Exception (tr ("Shader linking failed."));

      if (!_shader.bind ())
        throw Exception (tr ("Shader binding failed."));

      //
      // Init render data
      //
      qDebug () << "* Render data:";
      foreach (const Face& face, _model->getFaces ())
        {
          Q_ASSERT (face.getPoints ().size () == 3 && "Only triangles are supported.");

          foreach (const Face::Point& point, face.getPoints ())
            {
              _vertices.push_back (_model->getVertices ()[point.getVertexIndex ()] / 10.0);
              _normals.push_back (_model->getNormals ()[point.getNormalIndex ()]);

              qDebug () << "  v=" << _vertices.back () << ", " << _normals.back ();
            }
        }

      resizeGL (width (), height ());
    }

    /*
     * Resize GL widget
     */
    void Widget::resizeGL (int width, int height)
    {
      Q_UNUSED (width);
      Q_UNUSED (height);

#if 0
      qreal aspect = qreal (width) / qreal (qMax (height, 1));
      //const qreal zNear = 1, zFar = 7, fov = 45.0;
      const qreal zNear = 0.1, zFar = 90, fov = 90.0;

      _projection.setToIdentity ();
      //_projection.perspective (fov, aspect, zNear, zFar);
#endif
    }

    /*
     * Paint GL widget
     */
    void Widget::paintGL ()
    {
      glClearColor (.2f, .2f, .2f, 1.0f);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      int vertex_attr = _shader.attributeLocation ("vertex");
      int normal_attr = _shader.attributeLocation ("normal");
      int matrix_attr = _shader.attributeLocation ("matrix");

      qDebug () << _mvp;

      _shader.bind ();
      _shader.setUniformValue(matrix_attr, _projection * _mvp);

      _shader.enableAttributeArray (vertex_attr);
      _shader.setAttributeArray (vertex_attr, _vertices.constData ());

      _shader.enableAttributeArray (normal_attr);
      _shader.setAttributeArray (normal_attr, _normals.constData ());

      glDrawArrays (GL_TRIANGLES, 0, _vertices.size ());

      _shader.disableAttributeArray (normal_attr);
      _shader.disableAttributeArray (vertex_attr);

      _shader.release ();
    }

    void Widget::keyPressEvent (QKeyEvent* event)
    {
      const double step = 0.1;

      if (event->key () == Qt::Key_X)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _mvp.translate (QVector3D (-step, 0, 0));
          else
            _mvp.translate (QVector3D (+step, 0, 0));
        }
      else if (event->key () == Qt::Key_Y)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _mvp.translate (QVector3D (0, -step, 0));
          else
            _mvp.translate (QVector3D (0, +step, 0));
        }
      else if (event->key () == Qt::Key_Z)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _mvp.translate (QVector3D (0, 0, -step));
          else
            _mvp.translate (QVector3D (0, 0, +step));
        }
      else if (event->key () == Qt::Key_Plus)
        _mvp.scale (QVector3D (1.1f, 1.1f, 1.1f));
      else if (event->key () == Qt::Key_Minus)
        _mvp.scale (QVector3D (0.9f, 0.9f, 0.9f));

      update ();
    }

    void Widget::mousePressEvent (QMouseEvent* event)
    {
      _last_pos = event->pos ();
    }

    void Widget::mouseMoveEvent (QMouseEvent* event)
    {
      QPointF delta = event->pos () - _last_pos;
      _last_pos = event->pos ();

      if (event->buttons ().testFlag (Qt::LeftButton))
        {
          _mvp.rotate (delta.x (), QVector3D (0.0, 1.0, 0.0));
          _mvp.rotate (delta.y (), QVector3D (1.0, 0.0, 0.0));
        }

      update ();
    }

    void Widget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::wheelEvent (QWheelEvent* event)
    {
#if 0
      double factor = 0.1 * (event->angleDelta ().x () + event->angleDelta ().y ()) / (15 * 8);
      _mvp.scale (QVector3D (1.0 - factor, 1.0 - factor, 1.0 - factor));
#else
      double step = event->angleDelta ().y () > 0 ? 1.0 : -1.0;
      _mvp.translate (QVector3D (0.0, 0.0, step));
#endif

      update ();
    }


    //#**********************************************************************
    // CLASS HIP::GL::View
    //#**********************************************************************

    /*! Constructor */
    View::View (const Database::Database* database, const QString& model_path, QWidget* parent)
      : QWidget(parent),
        _ui     (new Ui::HIP_GL_View),
        _widget (0)
    {
      _ui->setupUi (this);
      _widget = Tools::addToParent (new Widget (model_path, _ui->_view_w));

      connect (database, &Database::Database::databaseChanged, this, &View::onDatabaseChanged);
    }

    /*! Destructor */
    View::~View ()
    {
      delete _ui;
    }

    /*! React on database changes */
    void View::onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data)
    {
      Q_UNUSED (data);

      if (reason == Database::Database::Reason::DATA)
        {
          Q_ASSERT (!data.isValid ());
        }
    }


  }
}
