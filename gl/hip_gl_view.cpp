/*
 * hip_gl_view.cpp - View displaying a GL scene
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGLView.h"
#include "HIPGLRenderable.h"
#include "HIPGLData.h"
#include "HIPGLPin.h"
#include "ui_hip_gl_view.h"

#include "core/HIPConfig.h"
#include "core/HIPException.h"
#include "core/HIPTools.h"

#include <QActionGroup>
#include <QCursor>
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
      void drawRenderable (const RenderablePtr& renderable, const RenderableParameters& parameters);
      float checkBounds (float lower, float value, float upper) const;

    private:
      Database::Database* _database;
      const Data* _data;
      Data _pin_data;

      QCursor _rotate_cursor;
      QCursor _rotate_y_cursor;

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
      QMatrix4x4 _camera_matrix;
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
        _pin_data          (Config::PIN_MODEL_FILE),
        _rotate_cursor     (QPixmap (Config::CURSOR_ROTATE)),
        _rotate_y_cursor   (QPixmap (Config::CURSOR_ROTATE_Y)),
        _pin               (),
        _vertex_attr       (-1),
        _normal_attr       (-1),
        _mvp_matrix_attr   (-1),
        _mv_matrix_attr    (-1),
        _n_matrix_attr     (-1),
        _texture_attr      (-1),
        _projection_matrix (),
        _camera_matrix     (),
        _view_matrix       (),
        _last_pos          (0, 0)
    {
      _pin_data.normalize ();
      _pin_data.scale (1.0 / 2.0);
      _pin = RenderablePtr (new Renderable (&_pin_data));

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

      _camera_matrix.setToIdentity ();
    }

    /*! Reset view */
    void Widget::resetView ()
    {
      if (!_model.isNull ())
        {
          Data::Cube cube = _model->getBoundingBox ();
          _view_matrix.setToIdentity ();
          _view_matrix.translate (0, 0, (cube.first + cube.second).z () / 2);

          _camera_matrix.setToIdentity ();
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
      _pin->initialize ();
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
          RenderableParameters model_parameters;

          QSet<QString> groups;
          if (!_database->getCurrentView ().isEmpty ())
            {
              foreach (const Database::View& view, _database->getViews ())
                if (view.getName () == _database->getCurrentView ())
                  model_parameters.setVisibleGroups (view.getGroups ().toSet ());
            }

          drawRenderable (_model, model_parameters);

          RenderableParameters pin_parameters;
          foreach (const Database::Point& point, _database->getPoints ())
            {
              pin_parameters.setPosition (point.getPosition ());
              pin_parameters.setTransparent (!point.getSelected ());
              drawRenderable (_pin, pin_parameters);
            }
        }
    }

    /*
     * Draw single renderable
     */
    void Widget::drawRenderable (const RenderablePtr& renderable, const RenderableParameters& parameters)
    {
      renderable->bind ();

      _shader.bind ();
      _shader.setUniformValue (_mvp_matrix_attr, _projection_matrix * _view_matrix * _camera_matrix);
      _shader.setUniformValue (_mv_matrix_attr, _view_matrix * _camera_matrix);
      _shader.setUniformValue (_n_matrix_attr, (_view_matrix * _camera_matrix).normalMatrix ());
      _shader.setUniformValue ("in_texture", 0);

      int offset = 0;

      _shader.enableAttributeArray (_vertex_attr);
      _shader.setAttributeBuffer (_vertex_attr, GL_FLOAT, offset, 3, renderable->getElementSize ());

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_normal_attr);
      _shader.setAttributeBuffer (_normal_attr, GL_FLOAT, offset, 3, renderable->getElementSize ());

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_texture_attr);
      _shader.setAttributeBuffer (_texture_attr, GL_FLOAT, offset, 2, renderable->getElementSize ());
      _shader.setUniformValue ("has_texture", renderable->hasTexture ());

      QMatrix4x4 model_matrix;
      model_matrix.translate (parameters.getPosition ());

      renderable->paint (_projection_matrix * _view_matrix * model_matrix, parameters);

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
        _camera_matrix.rotate (+5, _camera_matrix.inverted () * QVector3D (1, 0, 0));
      else if (event->key () == Qt::Key_Down)
        _camera_matrix.rotate (-5, _camera_matrix.inverted () * QVector3D (1, 0, 0));
      else if (event->key () == Qt::Key_Left)
        _camera_matrix.rotate (+5, _camera_matrix.inverted () * QVector3D (0, 1, 0));
      else if (event->key () == Qt::Key_Right)
        _camera_matrix.rotate (-5, _camera_matrix.inverted () * QVector3D (0, 1, 0));

      _database->emitViewChanged (qVariantFromValue (_view_matrix * _camera_matrix));

      update ();
    }

    void Widget::mousePressEvent (QMouseEvent* event)
    {
      _last_pos = event->pos ();

      if (event->buttons ().testFlag (Qt::LeftButton))
        {
          if (event->modifiers ().testFlag (Qt::ControlModifier))
            setCursor (_rotate_y_cursor);
          else
            setCursor (_rotate_cursor);
        }
      else if (event->buttons ().testFlag (Qt::MidButton))
        setCursor (Qt::SizeAllCursor);
    }

    void Widget::mouseMoveEvent (QMouseEvent* event)
    {
      QPointF delta = event->pos () - _last_pos;
      _last_pos = event->pos ();

      if (event->buttons ().testFlag (Qt::LeftButton))
        {
          if (event->modifiers ().testFlag (Qt::ControlModifier))
            {
              _camera_matrix.rotate (-delta.x (), _camera_matrix.inverted () * QVector3D (0, 0, 1));
            }
          else
            {
              _camera_matrix.rotate (delta.y (), _camera_matrix.inverted () * QVector3D (1, 0, 0));
              _camera_matrix.rotate (delta.x (), _camera_matrix.inverted () * QVector3D (0, 1, 0));
            }
        }
      else if (event->buttons ().testFlag (Qt::MidButton))
        {
          QVector3D translation (static_cast<float> (delta.x ()) / width (),
                                 static_cast<float> (-delta.y ()) / height (),
                                 0.0f);
          _camera_matrix.translate (translation);
        }

      _database->emitViewChanged (qVariantFromValue (_view_matrix * _camera_matrix));

      update ();
    }

    void Widget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
      unsetCursor ();
    }

    void Widget::wheelEvent (QWheelEvent* event)
    {
      _view_matrix.translate (0, 0, (event->delta () / 120.0) * 0.1);

      _database->emitViewChanged (qVariantFromValue (_view_matrix * _camera_matrix));

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
