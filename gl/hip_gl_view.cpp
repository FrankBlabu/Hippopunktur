/*
 * hip_gl_view.cpp - View displaying a GL scene
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGLView.h"
#include "HIPGLData.h"
#include "ui_hip_gl_view.h"

#include "core/HIPException.h"
#include "core/HIPTools.h"

#include <QActionGroup>
#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QSurfaceFormat>
#include <QToolBar>
#include <QWheelEvent>


namespace HIP {
  namespace GL {

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
        QVector<GLuint> _index_data;

        typedef QMap<Point, int> PointIndexMap;
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
      Widget (const Database::Database* database, QWidget* parent);
      virtual ~Widget ();

      void setData (const Data* data);

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
      void addVertex (VertexCollector* collector, const Point& point) const;
      float checkBounds (float lower, float value, float upper) const;

    private:
      const Database::Database* _database;
      const Data* _data;

      QOpenGLShaderProgram _shader;
      QOpenGLBuffer _vertex_buffer;
      QOpenGLBuffer _index_buffer;

      typedef QMap<QString, QOpenGLTexture*> TextureMap;
      TextureMap _textures;

      int _vertex_attr;
      int _normal_attr;
      int _color_attr;
      int _mvp_matrix_attr;
      int _mv_matrix_attr;
      int _n_matrix_attr;
      int _light_position_attr;
      int _texture_attr;

      QMatrix4x4 _model_matrix;
      QMatrix4x4 _view_matrix;

      QPointF _last_pos;
    };


    /*! Constructor */
    Widget::Widget (const Database::Database* database, QWidget* parent)
      : QOpenGLWidget (parent),
        _database            (database),
        _data                (0),
        _shader              (),
        _vertex_buffer       (QOpenGLBuffer::VertexBuffer),
        _index_buffer        (QOpenGLBuffer::IndexBuffer),
        _textures            (),
        _vertex_attr         (-1),
        _normal_attr         (-1),
        _color_attr          (-1),
        _mvp_matrix_attr     (-1),
        _mv_matrix_attr      (-1),
        _n_matrix_attr       (-1),
        _light_position_attr (-1),
        _texture_attr        (-1),
        _model_matrix        (),
        _view_matrix         (),
        _last_pos            (0, 0)
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
      for (TextureMap::const_iterator i = _textures.begin (); i != _textures.end (); ++i)
        delete i.value ();

      _index_buffer.destroy ();
      _vertex_buffer.destroy ();

      doneCurrent ();
    }

    /*
     * Set displayed model
     */
    void Widget::setData (const Data* data)
    {
      Q_ASSERT (data != 0);

      delete _data;
      _data = data;

      Data::Cube cube = _data->getBoundingBox ();
      _view_matrix.setToIdentity ();
      _view_matrix.translate (0, 0, (cube.first + cube.second).z () / 2);

      _model_matrix.setToIdentity ();
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

      foreach (const GroupPtr& group, _data->getGroups ())
        {
          if (!group->getMaterial ().isEmpty ())
            {
              const Material& material = _data->getMaterial (group->getMaterial ());
              if ( !material.getTexture ().isEmpty () &&
                   !_textures.contains (group->getMaterial ()) )
                {
                  QOpenGLTexture* texture = new QOpenGLTexture (Tools::loadResource<QImage> (material.getTexture ()).mirrored ());
                  texture->setMinificationFilter (QOpenGLTexture::Nearest);
                  texture->setMagnificationFilter (QOpenGLTexture::Linear);
                  texture->setWrapMode (QOpenGLTexture::Repeat);

                  _textures.insert (group->getMaterial (), texture);
                }
            }
        }

      //
      // Init render data
      //
      VertexCollector vertices;

      foreach (const GroupPtr& group, _data->getGroups ())
        {
          foreach (const Face& face, group->getFaces ())
            {
              Q_ASSERT (face.getPoints ().size () == 3);

              addVertex (&vertices, face.getPoints ()[0]);
              addVertex (&vertices, face.getPoints ()[1]);
              addVertex (&vertices, face.getPoints ()[2]);
            }
        }

      _vertex_buffer.create ();
      _vertex_buffer.bind ();
      _vertex_buffer.allocate (vertices._vertex_data.constData (), vertices._vertex_data.size () * sizeof (VertexData));

      _index_buffer.create ();
      _index_buffer.bind ();
      _index_buffer.allocate (vertices._index_data.constData (), vertices._index_data.size () * sizeof (GLuint));
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

      if (_data != 0)
        {
          QSet<QString> active_groups;
          if (!_database->getCurrentView ().isEmpty ())
            {
              foreach (const Database::View& view, _database->getViews ())
                if (view.getName () == _database->getCurrentView ())
                  active_groups = view.getGroups ().toSet ();
            }

          QMatrix4x4 projection;
          projection.perspective (45.0f /*fov*/, qreal (width ()) / qreal (height ()) /*aspect*/, 0.05f /*zNear*/, 20.0f /*zFar*/);

          _shader.bind ();
          _shader.setUniformValue (_mvp_matrix_attr, projection * _view_matrix * _model_matrix);
          _shader.setUniformValue (_mv_matrix_attr, _view_matrix * _model_matrix);
          _shader.setUniformValue (_n_matrix_attr, (_view_matrix * _model_matrix).normalMatrix ());
          _shader.setUniformValue (_light_position_attr, _view_matrix * QVector3D (0, 0, 1));

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

          int point_offset = 0;
          foreach (const GroupPtr& group, _data->getGroups ())
            {
              if (active_groups.isEmpty () || active_groups.contains (group->getName ()))
                {
                  QOpenGLTexture* texture = 0;
                  if (!group->getMaterial ().isEmpty ())
                    {
                      TextureMap::const_iterator pos = _textures.find (group->getMaterial ());
                      if (pos != _textures.end ())
                        texture = pos.value ();

                      const Material& material = _data->getMaterial (group->getMaterial ());

                      _shader.setUniformValue ("in_ambient_color", material.getAmbient ());
                      _shader.setUniformValue ("in_diffuse_color", material.getDiffuse ());
                      _shader.setUniformValue ("in_specular_color", material.getSpecular ());
                      _shader.setUniformValue ("in_specular_exponent", material.getSpecularExponent ());
                    }

                  if (texture != 0)
                    {
                      texture->bind ();
                      _shader.setUniformValue ("in_texture", 0);
                      _shader.setUniformValue ("has_texture", true);
                    }
                  else
                    _shader.setUniformValue ("has_texture", false);

                  glDrawElements (GL_TRIANGLES, group->getFaces ().size () * 3, GL_UNSIGNED_INT, (void*)(point_offset * sizeof (GLuint)));

                  if (texture != 0)
                    texture->release ();
                }

              point_offset += group->getFaces ().size () * 3;
            }

          _shader.disableAttributeArray (_texture_attr);
          _shader.disableAttributeArray (_color_attr);
          _shader.disableAttributeArray (_normal_attr);
          _shader.disableAttributeArray (_vertex_attr);

          _shader.release ();
        }
    }

    void Widget::keyPressEvent (QKeyEvent* event)
    {
      if (event->key () == Qt::Key_Plus)
        _view_matrix.translate (0, 0, -0.1f);
      else if (event->key () == Qt::Key_Minus)
        _view_matrix.translate (0, 0, +0.1f);
      else if (event->key () == Qt::Key_Up)
        _model_matrix.rotate (+5, _model_matrix.inverted () * QVector3D (1, 0, 0));
      else if (event->key () == Qt::Key_Down)
        _model_matrix.rotate (-5, _model_matrix.inverted () * QVector3D (1, 0, 0));
      else if (event->key () == Qt::Key_Left)
        _model_matrix.rotate (+5, _model_matrix.inverted () * QVector3D (0, 1, 0));
      else if (event->key () == Qt::Key_Right)
        _model_matrix.rotate (-5, _model_matrix.inverted () * QVector3D (0, 1, 0));

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
          if (event->modifiers ().testFlag (Qt::ControlModifier))
            {
              _model_matrix.rotate (-delta.x (), _model_matrix.inverted () * QVector3D (0, 0, 1));
            }
          else
            {
              _model_matrix.rotate (delta.y (), _model_matrix.inverted () * QVector3D (1, 0, 0));
              _model_matrix.rotate (delta.x (), _model_matrix.inverted () * QVector3D (0, 1, 0));
            }
        }
      else if (event->buttons ().testFlag (Qt::MidButton))
        {
          QVector3D translation (static_cast<float> (delta.x ()) / width (),
                                 static_cast<float> (-delta.y ()) / height (),
                                 0.0f);
          _model_matrix.translate (translation);
        }

      update ();
    }

    void Widget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::wheelEvent (QWheelEvent* event)
    {
      _view_matrix.translate (0, 0, (event->delta () / 120.0) * 0.1);
      update ();
    }

    /*
     * Add single vertex
     */
    void Widget::addVertex (VertexCollector* collector, const Point& point) const
    {
      VertexCollector::PointIndexMap::const_iterator pos = collector->_point_indices.find (point);
      if (pos == collector->_point_indices.end ())
        {
          QVector2D texture_point (0, 0);
          if (point.getTextureIndex () >= 0)
            texture_point = _data->getTextures ()[point.getTextureIndex ()];

          collector->_vertex_data.push_back (VertexData (_data->getVertices ()[point.getVertexIndex ()],
                                                         _data->getNormals ()[point.getNormalIndex ()],
                                                         QVector3D (1.0, 1.0, 1.0),
                                                         texture_point));
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
    View::View (Database::Database* database, QWidget* parent)
      : QWidget (parent),
        _ui           (new Ui::HIP_GL_View),
        _database     (database),
        _widget       (0),
        _toolbar      (0),
        _action_group (0)
    {
      _ui->setupUi (this);

      _widget = Tools::addToParent (new Widget (database, _ui->_view_w));
      _widget->setData (database->getModel ());

      connect (database, &Database::Database::databaseChanged, this, &View::onDatabaseChanged);

      updateToolBar ();
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
          updateToolBar ();
        }
    }

    /*! Update view switching buttons */
    void View::updateToolBar ()
    {
      delete _action_group;
      delete _toolbar;
      _action_view_map.clear ();

      _toolbar = Tools::addToParent (new QToolBar (_ui->_toolbar_frame_w));
      _toolbar->setOrientation (Qt::Vertical);
      _toolbar->setIconSize (QSize (16, 16));
      _toolbar->setToolButtonStyle (Qt::ToolButtonIconOnly);

      QAction* reset_action = new QAction (QIcon (), QString ("..."), _toolbar);
      reset_action->setToolTip (tr ("Reset view"));
      _toolbar->addAction (reset_action);

      connect (reset_action, SIGNAL (triggered (bool)), SLOT (onResetView ()));

      _toolbar->addSeparator ();

      _action_group = new QActionGroup (_toolbar);
      _action_group->setExclusive (true);

      foreach (const Database::View& view, _database->getViews ())
        {
          QAction* action = new QAction (QIcon (), QString ("..."), this);
          action->setToolTip (view.getName ());

          _toolbar->addAction (action);
          _action_group->addAction (action);

          if (_action_view_map.isEmpty ())
            action->setEnabled (true);

          _action_view_map.insert (action, view.getName ());

          connect (action, SIGNAL (triggered (bool)), SLOT (onSelectView ()));
        }

      QWidget* spacer = new QWidget;
      spacer->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);
      _toolbar->addWidget (spacer);
    }

    /*! Reset view */
    void View::onResetView ()
    {
    }

    /*! Select view */
    void View::onSelectView ()
    {
      QAction* action = qobject_cast<QAction*> (sender ());
      Q_ASSERT (action != 0);

      ActionViewMap::const_iterator pos = _action_view_map.find (action);
      Q_ASSERT (pos != _action_view_map.end ());

      _database->setCurrentView (pos.value ());
    }

  }
}
