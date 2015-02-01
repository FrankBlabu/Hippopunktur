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
#include <QOpenGLTexture>
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
        VertexData (const QVector3D& vertex, const QVector3D& normal, const QVector3D& color, const QVector3D& texture);

        QVector3D _vertex;
        QVector3D _normal;
        QVector3D _color;
        QVector2D _texture;
      };

      VertexData::VertexData (const QVector3D& vertex, const QVector3D& normal, const QVector3D& color, const QVector3D& texture)
        : _vertex  (vertex),
          _normal  (normal),
          _color   (color),
          _texture (texture)
      {
      }

    }


    //#**********************************************************************
    // CLASS HIP::GL::VertexCollector
    //#**********************************************************************

    namespace {

      struct VertexCollector
      {
        QVector<VertexData> _vertex_data;
        QVector<GLushort> _index_data;

        typedef QMap<Face::Point, int> PointIndexMap;
        PointIndexMap _point_indices;
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
      void addRotation (const QVector3D& delta);
      void addVertex (VertexCollector* collector, const Face::Point& point) const;
      float checkBounds (float lower, float value, float upper) const;

    private:
      Model* _model;

      QOpenGLShaderProgram _shader;
      QOpenGLBuffer _vertex_buffer;
      QOpenGLBuffer _index_buffer;
      QOpenGLTexture* _texture;

      int _vertex_attr;
      int _normal_attr;
      int _color_attr;
      int _mvp_matrix_attr;
      int _mv_matrix_attr;
      int _n_matrix_attr;
      int _light_position_attr;
      int _texture_attr;

      QMatrix4x4 _model_matrix;
      QVector3D _translation;
      QVector3D _rotation;

      QPointF _last_pos;
    };


    /*! Constructor */
    Widget::Widget (const QString& model_path, QWidget* parent)
      : QOpenGLWidget (parent),
        _model               (new Model (model_path)),
        _shader              (),
        _vertex_buffer       (QOpenGLBuffer::VertexBuffer),
        _index_buffer        (QOpenGLBuffer::IndexBuffer),
        _texture             (0),
        _vertex_attr         (-1),
        _normal_attr         (-1),
        _color_attr          (-1),
        _mvp_matrix_attr     (-1),
        _mv_matrix_attr      (-1),
        _n_matrix_attr       (-1),
        _light_position_attr (-1),
        _texture_attr        (-1),
        _model_matrix        (),
        _translation         (0, 0, 2),
        _rotation            (0, 0, 0),
        _last_pos            (0, 0)
    {
      setFocusPolicy (Qt::WheelFocus);

      QSurfaceFormat format;
      format.setDepthBufferSize (24);
      format.setStencilBufferSize (8);
      setFormat (format);

      Model::Cube cube = _model->getBoundingBox ();
      _model_matrix.translate (-(cube.first + cube.second) / 2);
    }

    /*! Destructor */
    Widget::~Widget ()
    {
      makeCurrent ();

      // Delete GL related structures
      delete _texture;
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
      _mvp_matrix_attr = _shader.uniformLocation ("in_mvp_matrix");
      _mv_matrix_attr = _shader.uniformLocation ("in_mv_matrix");
      _n_matrix_attr = _shader.uniformLocation ("in_n_matrix");
      _light_position_attr = _shader.uniformLocation ("in_light_position");
      _texture_attr = _shader.attributeLocation ("in_texture");

      _texture = new QOpenGLTexture (QImage (":/assets/models/horse/texture.png").mirrored ());
      _texture->setMinificationFilter (QOpenGLTexture::Nearest);
      _texture->setMagnificationFilter (QOpenGLTexture::Linear);
      _texture->setWrapMode (QOpenGLTexture::Repeat);

      //
      // Init render data
      //
      VertexCollector vertices;

      foreach (const Model::Group& group, _model->getGroups ())
        {
          foreach (const Face& face, group.getFaces ())
            {
              if (face.getPoints ().size () == 3)
                {
                  addVertex (&vertices, face.getPoints ()[0]);
                  addVertex (&vertices, face.getPoints ()[1]);
                  addVertex (&vertices, face.getPoints ()[2]);
                }
              else if (face.getPoints ().size () == 4)
                {
                  // XXX: Does not work correctly, why ?
                  addVertex (&vertices, face.getPoints ()[0]);
                  addVertex (&vertices, face.getPoints ()[1]);
                  addVertex (&vertices, face.getPoints ()[2]);

                  addVertex (&vertices, face.getPoints ()[2]);
                  addVertex (&vertices, face.getPoints ()[3]);
                  addVertex (&vertices, face.getPoints ()[1]);
                }
              else
                throw Exception (tr ("Only triangular or rectangular faces are supported."));
            }
        }

      _vertex_buffer.create ();
      _vertex_buffer.bind ();
      _vertex_buffer.allocate (vertices._vertex_data.constData (), vertices._vertex_data.size () * sizeof (VertexData));

      _index_buffer.create ();
      _index_buffer.bind ();
      _index_buffer.allocate (vertices._index_data.constData (), vertices._index_data.size () * sizeof (GLushort));
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
      camera.rotate (_rotation.y (), QVector3D (0.0, 1.0, 0.0));
      camera.rotate (_rotation.x (), QVector3D (1.0, 0.0, 0.0));

      QMatrix4x4 view_matrix;
      view_matrix.lookAt (camera * _translation, QVector3D (0, 0, 0), camera * QVector3D (0.0, 1.0, 0.0));

      _shader.bind ();
      _shader.setUniformValue (_mvp_matrix_attr, projection * _model_matrix * view_matrix);
      _shader.setUniformValue (_mv_matrix_attr, _model_matrix * view_matrix);
      _shader.setUniformValue (_n_matrix_attr, view_matrix.normalMatrix ());
      _shader.setUniformValue (_light_position_attr, camera * _translation);

      int offset = 0;

      _shader.enableAttributeArray (_vertex_attr);
      _shader.setAttributeBuffer (_vertex_attr, GL_FLOAT, offset, 3, sizeof (VertexData));

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_normal_attr);
      _shader.setAttributeBuffer (_normal_attr, GL_FLOAT, offset, 3, sizeof (VertexData));

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_color_attr);
      _shader.setAttributeBuffer (_color_attr, GL_FLOAT, offset, 3, sizeof (VertexData));

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_texture_attr);
      _shader.setAttributeBuffer (_texture_attr, GL_FLOAT, offset, 2, sizeof (VertexData));

      int number_of_points = 0;
      foreach (const Model::Group& group, _model->getGroups ())
        number_of_points += group.getFaces ().size () * 3;

      _texture->bind ();
      _shader.setUniformValue ("texture", 0);

      glDrawElements (GL_TRIANGLES, number_of_points, GL_UNSIGNED_SHORT, 0);

      _shader.disableAttributeArray (_texture_attr);
      _shader.disableAttributeArray (_color_attr);
      _shader.disableAttributeArray (_normal_attr);
      _shader.disableAttributeArray (_vertex_attr);

      _shader.release ();
    }

    void Widget::keyPressEvent (QKeyEvent* event)
    {
      if (event->key () == Qt::Key_Plus)
        _translation.setZ (_translation.z () * 0.9);
      else if (event->key () == Qt::Key_Minus)
        _translation.setZ (_translation.z () * 1.1);
      else if (event->key () == Qt::Key_Up)
        addRotation (QVector3D (+5, 0, 0));
      else if (event->key () == Qt::Key_Down)
        addRotation (QVector3D (-5, 0, 0));
      else if (event->key () == Qt::Key_Left)
        addRotation (QVector3D (0, +5, 0));
      else if (event->key () == Qt::Key_Right)
        addRotation (QVector3D (0, -5, 0));

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
        addRotation (QVector3D (-delta.y (), -delta.x (), 0.0));
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
      if (event->delta () < 0)
        _translation.setZ (_translation.z () * 1.1);
      else if (event->delta () > 0)
        _translation.setZ (_translation.z () * 0.9);

      update ();
    }

    /*!
     * Add rotation vector and checks bounds
     */
    void Widget::addRotation (const QVector3D& delta)
    {
      _rotation += delta;

      _rotation.setX (checkBounds (0, _rotation.x (), 360));
      _rotation.setY (checkBounds (0, _rotation.y (), 360));
      _rotation.setZ (checkBounds (0, _rotation.z (), 360));
    }

    /*
     * Add single vertex
     */
    void Widget::addVertex (VertexCollector* collector, const Face::Point& point) const
    {
      VertexCollector::PointIndexMap::const_iterator pos = collector->_point_indices.find (point);
      if (pos == collector->_point_indices.end ())
        {
          collector->_vertex_data.push_back (VertexData (_model->getVertices ()[point.getVertexIndex ()],
                                                         _model->getNormals ()[point.getNormalIndex ()],
                                                         QVector3D (random (), random (), random ()),
                                                         _model->getTextures ()[point.getTextureIndex ()]));
          collector->_point_indices.insert (point, collector->_vertex_data.size () - 1);
          collector->_index_data.push_back (collector->_vertex_data.size () - 1);
        }
      else
        collector->_index_data.push_back (pos.value ());
    }

    /*!
     * Check that the given value is in the bounds
     */
    float Widget::checkBounds (float lower, float value, float upper) const
    {
      while (value < lower)
        value += (upper - lower);
      while (value >= upper)
        value -= (upper - lower);

      return value;
    }


    //#**********************************************************************
    // CLASS HIP::GL::View
    //#**********************************************************************

    /*! Constructor */
    View::View (const Database::Database* database, QWidget* parent)
      : QWidget (parent),
        _ui     (new Ui::HIP_GL_View),
        _widget (0)
    {
      _ui->setupUi (this);
      _widget = Tools::addToParent (new Widget (database->getModel (), _ui->_view_w));

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
