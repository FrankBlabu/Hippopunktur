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
#include <QOpenGLBuffer>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#include <QWheelEvent>

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // Local functions
    //#**********************************************************************

    namespace {

      float random ()
      {
        return static_cast<float> (rand ()) / RAND_MAX;
      }

    }

    //#**********************************************************************
    // CLASS HIP::GL::VertexData
    //#**********************************************************************

    namespace {

      struct VertexData
      {
        VertexData () {}
        VertexData (const QVector3D& vertex, const QVector3D& normal, const QVector3D& color)
          : _vertex (vertex), _normal (normal), _color (color) {}

        QVector3D _vertex;
        QVector3D _normal;
        QVector3D _color;
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
      QOpenGLBuffer _vertex_buffer;
      QOpenGLBuffer _index_buffer;

      int _vertex_attr;
      int _normal_attr;
      int _color_attr;
      int _matrix_attr;

      QVector3D _translation;
      QVector3D _rotation;

      QPointF _last_pos;
    };


    /*! Constructor */
    Widget::Widget (const QString& model_path, QWidget* parent)
      : QOpenGLWidget (parent),
        _model         (new Model (model_path)),
        _shader        (),
        _vertex_buffer (QOpenGLBuffer::VertexBuffer),
        _index_buffer  (QOpenGLBuffer::IndexBuffer),
        _vertex_attr   (-1),
        _normal_attr   (-1),
        _color_attr    (-1),
        _matrix_attr   (-1),
        _translation   (0, 0, 5),
        _rotation      (0, 0, 0),
        _last_pos      (0, 0)
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
      _index_buffer.destroy ();
      _vertex_buffer.destroy ();

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

      _vertex_attr = _shader.attributeLocation ("in_vertex");
      _normal_attr = _shader.attributeLocation ("in_normal");
      _color_attr = _shader.attributeLocation ("in_color");
      _matrix_attr = _shader.uniformLocation ("in_matrix");

      //
      // Init render data
      //
      QVector<VertexData> vertex_data;

      foreach (const QVector3D& vertex, _model->getVertices ())
        vertex_data.push_back (VertexData (vertex,
                                           QVector3D (),
                                           QVector3D (random (), random (), random ())));

      _vertex_buffer.create ();
      _vertex_buffer.bind ();
      _vertex_buffer.allocate (vertex_data.constData (), vertex_data.size () * sizeof (VertexData));

      QVector<GLushort> index_data;

      foreach (const Face& face, _model->getFaces ())
        {
          Q_ASSERT (face.getPoints ().size () == 3 && "Only triangles are supported.");

          foreach (const Face::Point& point, face.getPoints ())
            index_data.push_back (point.getVertexIndex ());
        }

      _index_buffer.create ();
      _index_buffer.bind ();
      _index_buffer.allocate (index_data.constData (), index_data.size () * sizeof (GLushort));
    }

    /*
     * Resize GL widget
     */
    void Widget::resizeGL (int width, int height)
    {
      Q_UNUSED (width);
      Q_UNUSED (height);
    }

    /*
     * Paint GL widget
     */
    void Widget::paintGL ()
    {
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      QMatrix4x4 projection;
      projection.perspective (45.0f /*fov*/, qreal (width ()) / qreal (height ()) /*aspect*/, 0.05f /*zNear*/, 20.0f /*zFar*/);

      QMatrix4x4 camera;
      camera.rotate (_rotation.x (), QVector3D (1.0, 0.0, 0.0));
      camera.rotate (_rotation.y (), QVector3D (0.0, 1.0, 0.0));
      camera.translate (_translation);

      _shader.bind ();
      _shader.setUniformValue (_matrix_attr, projection * camera.inverted ());

      int offset = 0;

      _shader.enableAttributeArray (_vertex_attr);
      _shader.setAttributeBuffer (_vertex_attr, GL_FLOAT, offset, 3, sizeof (VertexData));

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_normal_attr);
      _shader.setAttributeBuffer (_normal_attr, GL_FLOAT, offset, 3, sizeof (VertexData));

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_color_attr);
      _shader.setAttributeBuffer (_color_attr, GL_FLOAT, offset, 3, sizeof (VertexData));

      glDrawElements (GL_TRIANGLES, _model->getFaces ().size () * 3, GL_UNSIGNED_SHORT, 0);

      _shader.disableAttributeArray (_color_attr);
      _shader.disableAttributeArray (_normal_attr);
      _shader.disableAttributeArray (_vertex_attr);

      _shader.release ();
    }

    void Widget::keyPressEvent (QKeyEvent* event)
    {
      Q_UNUSED (event);
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
          _rotation.setY (_rotation.y () - delta.x ());
          _rotation.setX (_rotation.x () - delta.y ());
        }
      else if (event->buttons ().testFlag (Qt::MidButton))
        {
          _translation.setX (_translation.x () - delta.x () / (width () / _translation.length ()));
          _translation.setY (_translation.y () + delta.y () / (height () / _translation.length ()));
        }

      update ();
    }

    void Widget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::wheelEvent (QWheelEvent* event)
    {
      float step = 0.08 * (event->angleDelta ().x () + event->angleDelta ().y ()) / (15 * 8);
      _translation.setZ (_translation.z () + step * _translation.length ());

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
