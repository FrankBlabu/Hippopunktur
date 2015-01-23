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
      virtual void keyPressEvent (QKeyEvent* event);

      virtual void mousePressEvent (QMouseEvent* event);
      virtual void mouseMoveEvent (QMouseEvent* event);
      virtual void mouseReleaseEvent (QMouseEvent* event);
      virtual void wheelEvent (QWheelEvent* event);

    private:
      double normalizeAngle (double angle) const;

    private:
      Model* _model;

      QOpenGLShaderProgram _shader;
      QMatrix4x4 _projection;
      QMatrix4x4 _mvp;

      QVector3D _translation;
      QVector3D _rotation;
      double _scaling;
      QPointF _last_pos;

      int _number_of_indices;
    };


    /*! Constructor */
    Widget::Widget (const QString& model_path, QWidget* parent)
      : QOpenGLWidget (parent),
        _model             (new Model (model_path)),
        _shader            (),
        _projection        (),
        _mvp               (),
        _translation       (0, 0, 0),
        _rotation          (0, 0, 0),
        _scaling           (1.0),
        _last_pos          (0, 0),
        _number_of_indices (0)
    {
      setFocusPolicy (Qt::WheelFocus);
    }

    /*! Destructor */
    Widget::~Widget ()
    {
      makeCurrent ();

      // Delete GL related structures

      doneCurrent ();

      delete _model;
    }

    void Widget::initializeGL ()
    {
      initializeOpenGLFunctions ();
      glClearColor (0, 0, 0, 1);

      glEnable (GL_DEPTH_TEST);
      glEnable (GL_CULL_FACE);
      glShadeModel (GL_SMOOTH);
      glEnable (GL_LIGHTING);
      glEnable (GL_LIGHT0);

      static GLfloat lightPosition[4] = { 0, 0, 10, 1.0 };
      glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);

      // XXX: Horse model setup
      _translation = QVector3D (-0.2f, -3.1f, -16.7f);
      _rotation = QVector3D (-177, 215, -87);
      _scaling = 0.165;

#if 0

      //
      // Init shaders
      //
      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Vertex, ":/gl/VertexShader.glsl"))
        throw Exception (tr ("Unable to initialize vertex shader."));

      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Fragment, ":/gl/FragmentShader.glsl"))
        throw Exception (tr ("Unable to initialize fragment shader."));

      if (!_shader.link ())
        throw Exception (tr ("Shader setup failed."));

      if (!_shader.bind ())
        throw Exception (tr ("Shader setup failed."));
#endif

      glEnable (GL_DEPTH_TEST);
      glEnable (GL_CULL_FACE);
    }

    void Widget::resizeGL (int width, int height)
    {
      int side = qMin (width, height);

      glViewport ((width - side) / 2, (height - side) / 2, side, side);
      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      glOrtho (-2, +2, -2, +2, 1.0, 15.0);
      glMatrixMode (GL_MODELVIEW);
    }

    void Widget::paintGL ()
    {
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glLoadIdentity ();
      glScalef (_scaling, _scaling, _scaling);
      glTranslatef (_translation.x (), _translation.y (), _translation.z ());
      glRotatef (_rotation.x (), 1.0, 0.0, 0.0);
      glRotatef (_rotation.y (), 0.0, 1.0, 0.0);
      glRotatef (_rotation.z (), 0.0, 0.0, 1.0);

      foreach (const Face& face, _model->getFaces ())
        {
          const QList<Face::Point>& points = face.getPoints ();

          if (points.size () == 4)
            glBegin (GL_QUADS);
          else if (points.size () == 3)
            glBegin (GL_TRIANGLES);
          else
            throw Exception (tr ("Only 3 or 4 points per vertex supported."));

          glNormal3f (_model->getNormals ()[points.front ().getNormalIndex () - 1].x (),
              _model->getNormals ()[points.front ().getNormalIndex () - 1].y (),
              _model->getNormals ()[points.front ().getNormalIndex () - 1].z ());

          foreach (const Face::Point& point, face.getPoints ())
            {
              glVertex3f (_model->getVertices ()[point.getVertexIndex () - 1].x (),
                  _model->getVertices ()[point.getVertexIndex () - 1].y (),
                  _model->getVertices ()[point.getVertexIndex () - 1].z ());
            }

          glEnd ();
        }
    }

    void Widget::keyPressEvent (QKeyEvent* event)
    {
      const double step = 0.1;

      if (event->key () == Qt::Key_X)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _translation.setX (_translation.x () - step);
          else
            _translation.setX (_translation.x () + step);
        }
      else if (event->key () == Qt::Key_Y)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _translation.setY (_translation.y () - step);
          else
            _translation.setY (_translation.y () + step);
        }
      else if (event->key () == Qt::Key_Z)
        {
          if (event->modifiers ().testFlag (Qt::ShiftModifier))
            _translation.setZ (_translation.z () - step);
          else
            _translation.setZ (_translation.z () + step);
        }
      else if (event->key () == Qt::Key_Plus)
        _scaling *= 1.1;
      else if (event->key () == Qt::Key_Minus)
        _scaling /= 1.1;

      update ();
    }

    void Widget::mousePressEvent (QMouseEvent* event)
    {
      _last_pos = event->pos ();
    }

    void Widget::mouseMoveEvent (QMouseEvent* event)
    {
      QPointF delta = event->pos () - _last_pos;

      if (event->buttons ().testFlag (Qt::LeftButton))
        {
          if (event->modifiers ().testFlag (Qt::ControlModifier))
            {
              _rotation.setX (normalizeAngle (_rotation.x () - delta.y ()));
              _rotation.setZ (normalizeAngle (_rotation.z () - delta.x ()));
            }
          else
            {
              _rotation.setX (normalizeAngle (_rotation.x () - delta.y ()));
              _rotation.setY (normalizeAngle (_rotation.y () - delta.x ()));
            }
        }

      _last_pos = event->pos ();
      update ();
    }

    void Widget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
    }

    void Widget::wheelEvent (QWheelEvent* event)
    {
      Q_UNUSED (event);
    }

    double Widget::normalizeAngle (double angle) const
    {
        while (angle < 0)
          angle += 360 * 16;
        while (angle > 360)
          angle -= 360 * 16;

        return angle;
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
