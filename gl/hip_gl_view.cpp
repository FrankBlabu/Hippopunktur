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

    namespace {

      static GLfloat cube_vertices[] = {
        -0.5, 0.5, 0.5,
        0.5,-0.5,0.5,
        -0.5,-0.5,0.5,
        0.5, -0.5, 0.5,
        -0.5,0.5,0.5,
        0.5,0.5,0.5,
        -0.5, -0.5, -0.5,
        0.5,-0.5,-0.5,
        -0.5,0.5,-0.5,
        0.5, 0.5, -0.5,
        -0.5,0.5,-0.5,
        0.5,-0.5,-0.5,

        0.5, -0.5, -0.5,
        0.5,-0.5,0.5,
        0.5,0.5,-0.5,
        0.5, 0.5, 0.5,
        0.5,0.5,-0.5,
        0.5,-0.5,0.5,
        -0.5, 0.5, -0.5,
        -0.5,-0.5,0.5,
        -0.5,-0.5,-0.5,
        -0.5, -0.5, 0.5,
        -0.5,0.5,-0.5,
        -0.5,0.5,0.5,

        0.5, 0.5,  -0.5,
        -0.5, 0.5,  0.5,
        -0.5,  0.5,  -0.5,
        -0.5,  0.5,  0.5,
        0.5,  0.5,  -0.5,
        0.5, 0.5,  0.5,
        -0.5,  -0.5, -0.5,
        -0.5, -0.5, 0.5,
        0.5, -0.5, -0.5,
        0.5, -0.5, 0.5,
        0.5,  -0.5, -0.5,
        -0.5,  -0.5, 0.5
      };

      static GLfloat cube_normals[] = {

        0,0,-1,
        0,0,-1,
        0,0,-1,
        0,0,-1,
        0,0,-1,
        0,0,-1,
        0,0,1,
        0,0,1,
        0,0,1,
        0,0,1,
        0,0,1,
        0,0,1,

        -1,0,0,
        -1,0,0,
        -1,0,0,
        -1,0,0,
        -1,0,0,
        -1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,

        0,-1,0,
        0,-1,0,
        0,-1,0,
        0,-1,0,
        0,-1,0,
        0,-1,0,
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0
      };

    }

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
      QMatrix4x4 _rotation;
      QVector3D _translation;
      float _scaling;

      QVector<QVector3D> _vertices;
      QVector<QVector3D> _normals;

      int _vertex_attr;
      int _normal_attr;
      int _matrix_attr;

      QPointF _last_pos;
    };


    /*! Constructor */
    Widget::Widget (const QString& model_path, QWidget* parent)
      : QOpenGLWidget (parent),
        _model       (new Model (model_path)),
        _shader      (),
        _projection  (),
        _rotation    (),
        _translation (0.0, 0.0, 0.0),
        _scaling     (1.0),
        _vertices    (),
        _normals     (),
        _vertex_attr (-1),
        _normal_attr (-1),
        _matrix_attr (-1),
        _last_pos    (0, 0)
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

      glClearColor (.2f, .2f, .2f, 1.0f);
      glCullFace (GL_FRONT);
      glEnable (GL_CULL_FACE);
      glEnable (GL_DEPTH_TEST);

      //
      // Init shaders
      //
      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Vertex, ":/gl/VertexShader.glsl"))
        throw Exception (tr ("Unable to initialize vertex shader: %1")
                         .arg (_shader.log ()));

      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Fragment, ":/gl/FragmentShader.glsl"))
        throw Exception (tr ("Unable to initialize fragment shader: %1")
                         .arg (_shader.log ()));

      if (!_shader.link ())
        throw Exception (tr ("Shader linking failed: %1")
                         .arg (_shader.log ()));

      _vertex_attr = _shader.attributeLocation ("vertex");
      _normal_attr = _shader.attributeLocation ("normal");
      _matrix_attr = _shader.uniformLocation ("matrix");

      //
      // Init render data
      //
      _vertices.clear ();
      _normals.clear ();

      foreach (const Face& face, _model->getFaces ())
        {
          Q_ASSERT (face.getPoints ().size () == 3 && "Only triangles are supported.");

          foreach (const Face::Point& point, face.getPoints ())
            {
              _vertices.push_back (_model->getVertices ()[point.getVertexIndex ()]);
              _normals.push_back (_model->getNormals ()[point.getNormalIndex ()]);
            }
        }
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
      _projection.perspective (fov, aspect, zNear, zFar);
#endif
    }

    /*
     * Paint GL widget
     */
    void Widget::paintGL ()
    {
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      QMatrix4x4 mvp = _rotation;
      mvp.translate (_translation);
      mvp.scale (_scaling);

      _shader.bind ();
      _shader.setUniformValue (_matrix_attr, _projection * mvp);

      _shader.setAttributeArray (_vertex_attr, _vertices.constData ());
      _shader.setAttributeArray (_normal_attr, _normals.constData ());

      _shader.enableAttributeArray (_vertex_attr);
      _shader.enableAttributeArray (_normal_attr);

      glDrawArrays (GL_TRIANGLES, 0, _vertices.size ());

      _shader.disableAttributeArray (_normal_attr);
      _shader.disableAttributeArray (_vertex_attr);

      _shader.release ();
    }

    void Widget::keyPressEvent (QKeyEvent* event)
    {
      const double step = 0.1;

      if (event->key () == Qt::Key_X)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _translation += QVector3D (-step, 0, 0);
          else
            _translation += QVector3D (+step, 0, 0);
        }
      else if (event->key () == Qt::Key_Y)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _translation += QVector3D (0, -step, 0);
          else
            _translation += QVector3D (0, +step, 0);
        }
      else if (event->key () == Qt::Key_Z)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _translation += QVector3D (0, 0, -step);
          else
            _translation += QVector3D (0, 0, +step);
        }
      else if (event->key () == Qt::Key_Plus)
        _scaling *= 1.1f;
      else if (event->key () == Qt::Key_Minus)
        _scaling *= 0.9f;

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
          _rotation.rotate (-delta.x (), QVector3D (0.0, 1.0, 0.0));
          _rotation.rotate (-delta.y (), QVector3D (1.0, 0.0, 0.0));
        }
      else if (event->buttons ().testFlag (Qt::MiddleButton))
        _translation += QVector3D (+delta.x () * _scaling / width (),
                                   -delta.y () * _scaling / height (), 0.0);

      update ();
    }

    void Widget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::wheelEvent (QWheelEvent* event)
    {
      _scaling *= 1.0 - 0.1 * (event->angleDelta ().x () + event->angleDelta ().y ()) / (15 * 8);
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
