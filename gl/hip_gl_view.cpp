/*
 * hip_gl_view.cpp - View displaying a GL scene
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGLView.h"
#include "HIPGLData.h"
#include "HIPGLPin.h"
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
        VertexData (const QVector3D& vertex, const QVector3D& normal, const QVector3D& texture);

        QVector3D _vertex;
        QVector3D _normal;
        QVector2D _texture;
      };

      VertexData::VertexData (const QVector3D& vertex, const QVector3D& normal, const QVector3D& texture)
        : _vertex  (vertex),
          _normal  (normal),
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
    // CLASS HIP::GL::Renderable
    //#**********************************************************************

    /*
     * Renderable object
     */
    class Renderable
    {
      public:
        Renderable (const Data* data);
        ~Renderable ();

        void initialize ();
        void paint (const QMatrix4x4& view, const QSet<QString>& groups);

        void bind ();
        void release ();

        bool hasTexture () const { return _has_texture; }
        Data::Cube getBoundingBox () const { return _data->getBoundingBox (); }

      private:
        void addVertex (VertexCollector* collector, const Point& point) const;
        void setLightParameter (uint parameter, const QVector3D& value);

      private:
        const Data* _data;
        bool _has_texture;

        QOpenGLBuffer _vertex_buffer;
        QOpenGLBuffer _index_buffer;

        typedef QMap<QString, QOpenGLTexture*> TextureMap;
        TextureMap _textures;
    };

    typedef QSharedPointer<Renderable> RenderablePtr;

    /*! Constructor */
    Renderable::Renderable (const Data* data)
      : _data          (data),
        _has_texture   (false),
        _vertex_buffer (QOpenGLBuffer::VertexBuffer),
        _index_buffer  (QOpenGLBuffer::IndexBuffer),
        _textures      ()
    {
    }

    /*! Destructor */
    Renderable::~Renderable ()
    {
      for (TextureMap::const_iterator i = _textures.begin (); i != _textures.end (); ++i)
        delete i.value ();

      _index_buffer.destroy ();
      _vertex_buffer.destroy ();
    }

    /*! Initialize for drawing */
    void Renderable::initialize ()
    {
      if (_data != 0)
        {
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
                      _has_texture = true;
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
          _vertex_buffer.release ();

          _index_buffer.create ();
          _index_buffer.bind ();
          _index_buffer.allocate (vertices._index_data.constData (), vertices._index_data.size () * sizeof (GLuint));
          _index_buffer.release ();
        }
    }

    /*! Bind structures */
    void Renderable::bind ()
    {
      _vertex_buffer.bind ();
      _index_buffer.bind ();
    }

    /*! Release structures */
    void Renderable::release ()
    {
      _index_buffer.release ();
      _vertex_buffer.release ();
    }

    /*! Paint renderable */
    void Renderable::paint (const QMatrix4x4& view, const QSet<QString>& groups)
    {
      int point_offset = 0;
      foreach (const GroupPtr& group, _data->getGroups ())
        {
          if (groups.isEmpty () || groups.contains (group->getName ()))
            {
              QOpenGLTexture* texture = 0;
              if (!group->getMaterial ().isEmpty ())
                {
                  TextureMap::const_iterator pos = _textures.find (group->getMaterial ());
                  if (pos != _textures.end ())
                    texture = pos.value ();

                  const Material& material = _data->getMaterial (group->getMaterial ());

                  setLightParameter (GL_AMBIENT, material.getAmbient ());
                  setLightParameter (GL_DIFFUSE, material.getDiffuse ());
                  setLightParameter (GL_SPECULAR, material.getSpecular ());
                  setLightParameter (GL_POSITION, view * QVector3D (0, 0, 100));
                  glLightf (GL_LIGHT0, GL_SPOT_EXPONENT, material.getSpecularExponent ());
                  glEnable (GL_LIGHT0);
                }

              if (texture != 0)
                texture->bind ();

              glDrawElements (GL_TRIANGLES, group->getFaces ().size () * 3, GL_UNSIGNED_INT, (void*)(point_offset * sizeof (GLuint)));

              if (texture != 0)
                texture->release ();
            }

          point_offset += group->getFaces ().size () * 3;
        }
    }

    /*
     * Add single vertex to data vectors
     */
    void Renderable::addVertex (VertexCollector* collector, const Point& point) const
    {
      VertexCollector::PointIndexMap::const_iterator pos = collector->_point_indices.find (point);
      if (pos == collector->_point_indices.end ())
        {
          QVector2D texture_point (0, 0);
          if (point.getTextureIndex () >= 0)
            texture_point = _data->getTextures ()[point.getTextureIndex ()];

          collector->_vertex_data.push_back (VertexData (_data->getVertices ()[point.getVertexIndex ()],
                                                         _data->getNormals ()[point.getNormalIndex ()],
                                                         texture_point));
          collector->_point_indices.insert (point, collector->_vertex_data.size () - 1);
          collector->_index_data.push_back (collector->_vertex_data.size () - 1);
        }
      else
        collector->_index_data.push_back (pos.value ());
    }

    /*!
     * Set single vector based light parameter
     */
    void Renderable::setLightParameter (uint parameter, const QVector3D& value)
    {
      float array[] = {value.x (), value.y (), value.z (), 1.0f};
      glLightfv (GL_LIGHT0, parameter, array);
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
      Widget (Database::Database* database, QWidget* parent);
      virtual ~Widget ();

      void setData (const Data* data);
      void resetView ();

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
      void drawRenderable (const RenderablePtr& renderable, const QSet<QString>& groups);
      float checkBounds (float lower, float value, float upper) const;

    private:
      Database::Database* _database;
      const Data* _data;
      Data _pin_data;

      QOpenGLShaderProgram _shader;

      RenderablePtr _model;
      RenderablePtr _pin;

      int _vertex_attr;
      int _normal_attr;
      int _mvp_matrix_attr;
      int _mv_matrix_attr;
      int _n_matrix_attr;
      int _texture_attr;

      QMatrix4x4 _projection_matrix;
      QMatrix4x4 _model_matrix;
      QMatrix4x4 _view_matrix;

      QPointF _last_pos;
    };


    /*! Constructor */
    Widget::Widget (Database::Database* database, QWidget* parent)
      : QOpenGLWidget (parent),
        _database          (database),
        _data              (0),
        _shader            (),
        _model             (),
        _pin_data          ("assets/models/pin/pin.obj"),
        _pin               (new Renderable (&_pin_data)),
        _vertex_attr       (-1),
        _normal_attr       (-1),
        _mvp_matrix_attr   (-1),
        _mv_matrix_attr    (-1),
        _n_matrix_attr     (-1),
        _texture_attr      (-1),
        _projection_matrix (),
        _model_matrix      (),
        _view_matrix       (),
        _last_pos          (0, 0)
    {
      setFocusPolicy (Qt::WheelFocus);
      setContextMenuPolicy (Qt::NoContextMenu);

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
      _model.reset ();
      _pin.reset ();

      doneCurrent ();
    }

    /*
     * Set displayed model
     */
    void Widget::setData (const Data* data)
    {
      Q_ASSERT (data != 0);

      _model = RenderablePtr (new Renderable (data));

      Data::Cube cube = _model->getBoundingBox ();
      _view_matrix.setToIdentity ();
      _view_matrix.translate (0, 0, (cube.first + cube.second).z () / 2);

      _model_matrix.setToIdentity ();
    }

    /*! Reset view */
    void Widget::resetView ()
    {
      if (!_model.isNull ())
        {
          Data::Cube cube = _model->getBoundingBox ();
          _view_matrix.setToIdentity ();
          _view_matrix.translate (0, 0, (cube.first + cube.second).z () / 2);

          _model_matrix.setToIdentity ();
        }
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
      Q_ASSERT (_vertex_attr >= 0);

      _normal_attr = _shader.attributeLocation ("in_normal");
      Q_ASSERT (_normal_attr >= 0);

      _mvp_matrix_attr = _shader.uniformLocation ("in_mvp_matrix");
      Q_ASSERT (_mvp_matrix_attr >= 0);

      _mv_matrix_attr = _shader.uniformLocation ("in_mv_matrix");
      Q_ASSERT (_mv_matrix_attr >= 0);

      _n_matrix_attr = _shader.uniformLocation ("in_n_matrix");
      Q_ASSERT (_n_matrix_attr >= 0);

      _texture_attr = _shader.attributeLocation ("in_texture");
      Q_ASSERT (_texture_attr >= 0);

      _model->initialize ();
    }

    /*
     * Resize GL widget
     */
    void Widget::resizeGL (int width, int height)
    {
      _projection_matrix = QMatrix4x4 ();
      _projection_matrix.perspective (45.0f /*fov*/, qreal (width) / qreal (height) /*aspect*/, 0.05f /*zNear*/, 20.0f /*zFar*/);
    }

    /*
     * Paint GL widget
     */
    void Widget::paintGL ()
    {
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if (!_model.isNull ())
        {
          QSet<QString> groups;
          if (!_database->getCurrentView ().isEmpty ())
            {
              foreach (const Database::View& view, _database->getViews ())
                if (view.getName () == _database->getCurrentView ())
                  groups = view.getGroups ().toSet ();
            }


          drawRenderable (_model, groups);
#if 0
          foreach (const Database::Point& point, _database->getPoints ())
            if (point.getSelected ())
              _pin->draw (_projection * _view_matrix * _model_matrix, point.getPosition (), point.getColor (), 0.005);
            else
              _pin->draw (_projection * _view_matrix * _model_matrix, point.getPosition (), Qt::darkGray, 0.002);
#endif
        }
    }

    /*
     * Draw single renderable
     */
    void Widget::drawRenderable (const RenderablePtr& renderable, const QSet<QString>& groups)
    {
      renderable->bind ();

      _shader.bind ();
      _shader.setUniformValue (_mvp_matrix_attr, _projection_matrix * _view_matrix * _model_matrix);
      _shader.setUniformValue (_mv_matrix_attr, _view_matrix * _model_matrix);
      _shader.setUniformValue (_n_matrix_attr, (_view_matrix * _model_matrix).normalMatrix ());
      _shader.setUniformValue ("in_texture", 0);

      int offset = 0;

      _shader.enableAttributeArray (_vertex_attr);
      _shader.setAttributeBuffer (_vertex_attr, GL_FLOAT, offset, 3, sizeof (VertexData));

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_normal_attr);
      _shader.setAttributeBuffer (_normal_attr, GL_FLOAT, offset, 3, sizeof (VertexData));

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_texture_attr);
      _shader.setAttributeBuffer (_texture_attr, GL_FLOAT, offset, 2, sizeof (VertexData));
      _shader.setUniformValue ("has_texture", _model->hasTexture ());

      renderable->paint (_projection_matrix * _view_matrix, groups);

      _shader.disableAttributeArray (_texture_attr);
      _shader.disableAttributeArray (_normal_attr);
      _shader.disableAttributeArray (_vertex_attr);

      _shader.release ();

      renderable->release ();
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

      _database->emitViewChanged (qVariantFromValue (_view_matrix * _model_matrix));

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

      _database->emitViewChanged (qVariantFromValue (_view_matrix * _model_matrix));

      update ();
    }

    void Widget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::wheelEvent (QWheelEvent* event)
    {
      _view_matrix.translate (0, 0, (event->delta () / 120.0) * 0.1);

      _database->emitViewChanged (qVariantFromValue (_view_matrix * _model_matrix));

      update ();
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
      else if (reason == Database::Database::Reason::SELECTION)
        _widget->update ();
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
      _widget->resetView ();
    }

    /*! Select view */
    void View::onSelectView ()
    {
      QAction* action = qobject_cast<QAction*> (sender ());
      Q_ASSERT (action != 0);

      ActionViewMap::const_iterator pos = _action_view_map.find (action);
      Q_ASSERT (pos != _action_view_map.end ());

      _database->setCurrentView (pos.value ());
      update ();
    }

  }
}
