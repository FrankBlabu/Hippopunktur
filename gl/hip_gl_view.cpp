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

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QQuaternion>
#include <QPainter>
#include <QPixmap>
#include <QWheelEvent>


namespace HIP {
  namespace GL {

    //#**********************************************************************
    // CLASS HIP::GL::VertexData
    //#**********************************************************************

    struct VertexData
    {
      VertexData ()
        : _position (0, 0, 0), _texture (0, 0) {}
      VertexData (const QVector3D& position, const QVector2D& texture)
        : _position (position), _texture (texture) {}

      QVector3D _position;
      QVector2D _texture;
    };


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
      virtual void mousePressEvent (QMouseEvent* event);
      virtual void mouseMoveEvent (QMouseEvent* event);
      virtual void mouseReleaseEvent (QMouseEvent* event);
      virtual void wheelEvent (QWheelEvent* event);

    private:
      Model* _model;

      QOpenGLShaderProgram _shader;
      QOpenGLTexture* _texture;
      QMatrix4x4 _projection;
      QMatrix4x4 _mvp;

      QOpenGLBuffer _array_buffer;
      QOpenGLBuffer _index_buffer;

      int _number_of_indices;
    };


    /*! Constructor */
    Widget::Widget (const QString& model_path, QWidget* parent)
      : QOpenGLWidget (parent),
        _model             (new Model (model_path)),
        _shader            (),
        _texture           (0),
        _projection        (),
        _mvp               (),
        _array_buffer      (),
        _index_buffer      (QOpenGLBuffer::IndexBuffer),
        _number_of_indices (0)
    {
      _mvp.translate (0.0, 0.0, -5.0);
    }

    /*! Destructor */
    Widget::~Widget ()
    {
      makeCurrent ();

      // Delete GL related structures
      delete _texture;

      doneCurrent ();

      delete _model;
    }

    void Widget::initializeGL ()
    {
      initializeOpenGLFunctions ();
      glClearColor (0, 0, 0, 1);

      _array_buffer.create ();
      _index_buffer.create ();

      //
      // Init shaders
      //
      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Vertex, ":/gl/vshader.glsl"))
        throw Exception (tr ("Unable to initialize vertex shader."));

      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Fragment, ":/gl/fshader.glsl"))
        throw Exception (tr ("Unable to initialize fragment shader."));

      if (!_shader.link ())
        throw Exception (tr ("Shader setup failed."));

      if (!_shader.bind ())
        throw Exception (tr ("Shader setup failed."));

      //
      // Init textures
      //
      QPixmap texture (64, 64);
      {
        QPainter painter (&texture);
        painter.fillRect (texture.rect (), Qt::lightGray);
      }

      _texture = new QOpenGLTexture (texture.toImage ());
      _texture->setMinificationFilter (QOpenGLTexture::Nearest);
      _texture->setMagnificationFilter (QOpenGLTexture::Linear);
      _texture->setWrapMode (QOpenGLTexture::Repeat);

      glEnable (GL_DEPTH_TEST);
      glEnable (GL_CULL_FACE);

      //
      // Setup array buffer (vertices)
      //
      VertexData* vertices = new VertexData[_model->getVertices ().size ()];
      for (int i=0; i < _model->getVertices ().size (); ++i)
        vertices[i] = VertexData (_model->getVertices ()[i], QVector2D (0, 0));

      _array_buffer.bind ();
      _array_buffer.allocate (vertices, _model->getVertices ().size () * sizeof (VertexData));

      delete[] vertices;

      //
      // Setup index buffer (faces)
      //
      QVector<int> index_v;
      for (int i=0; i < _model->getFaces ().size (); ++i)
        {
          const Face& face = _model->getFaces ()[i];
          Q_ASSERT (!face.getPoints ().isEmpty ());

          if (i > 0)
            index_v.push_back (face.getPoints ().front ().getVertexIndex ());

          foreach (const Face::Point& point, face.getPoints ())
            index_v.push_back (point.getVertexIndex ());

          if (i + 1 < _model->getFaces ().size ())
            index_v.push_back (face.getPoints ().back ().getVertexIndex ());
        }

      _number_of_indices = index_v.size ();

      GLushort* indices = new GLushort[_number_of_indices];
      for (int i=0; i < index_v.size (); ++i)
        indices[i] = index_v.at (i);

      _index_buffer.bind ();
      _index_buffer.allocate (indices, _number_of_indices * sizeof (GLushort));

      delete[] indices;
    }

    void Widget::resizeGL (int width, int height)
    {
      qreal aspect = qreal (width) / qreal (height ? height : 1);

      _projection.setToIdentity ();
      _projection.perspective (45.0, aspect, 3.0, 7.0);
    }

    void Widget::paintGL ()
    {
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if (_texture != 0)
        _texture->bind ();

      _shader.setUniformValue ("mvp_matrix", _projection * _mvp);
      _shader.setUniformValue ("texture", 0);

      _array_buffer.bind ();
      _index_buffer.bind ();

      quintptr offset = 0;

      //
      // Tell OpenGL programmable pipeline how to locate vertex position data
      //
      int vertex_location = _shader.attributeLocation ("a_position");
      _shader.enableAttributeArray (vertex_location);
      _shader.setAttributeBuffer (vertex_location, GL_FLOAT, offset, 3, sizeof (VertexData));
      offset += sizeof (QVector3D);

      //
      // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
      //
      int texcoord_location = _shader.attributeLocation ("a_texcoord");
      _shader.enableAttributeArray (texcoord_location);
      _shader.setAttributeBuffer (texcoord_location, GL_FLOAT, offset, 2, sizeof (VertexData));

      //
      // Draw cube geometry using indices from VBO 1
      //
      glDrawElements (GL_TRIANGLE_STRIP, _number_of_indices, GL_UNSIGNED_SHORT, 0);
    }

    void Widget::mousePressEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::mouseMoveEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::wheelEvent (QWheelEvent* event)
    {
      Q_UNUSED (event);

      //double scaling = 0.1 * (event->angleDelta ().x () + event->angleDelta ().y ()) / (15 * 8);
      _mvp.rotate (QQuaternion (0.1f, 1.0f, 0.0f, 0.0f));
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
      if (reason == Database::Database::Reason::DATA)
        {
          Q_ASSERT (!data.isValid ());
        }
    }


  }
}
