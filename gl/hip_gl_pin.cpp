/*
 * hip_gl_painter.cpp - GL based pin
 *
 * Frank Blankenburg, Feb. 2015
 */

#include "HIPGLPin.h"
#include "core/HIPException.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // CLASS HIP::GL::PinData
    //#**********************************************************************

    namespace {

      struct PinData
      {
        PinData () {}
        PinData (const QVector3D& vertex)
          : _vertex (vertex) {}

        QVector3D _vertex;
      };

    }


    //#**********************************************************************
    // CLASS HIP::GL::PinImpl
    //#**********************************************************************

    /*!
     * Pin data keeping class
     */
    class PinImpl
    {
    public:
      PinImpl ();
      ~PinImpl ();

      void draw (const QMatrix4x4& mvp,
                 const QVector3D& position,
                 const QColor& color,
                 double radius);

    private:
      QOpenGLShaderProgram _shader;
      QOpenGLBuffer _vertex_buffer;
      QOpenGLBuffer _index_buffer;

      int _vertex_attr;
      int _mvp_attr;
      int _color_attr;

      int _number_of_points;
    };

    /*! Constructor */
    PinImpl::PinImpl ()
      : _shader           (),
        _vertex_buffer    (QOpenGLBuffer::VertexBuffer),
        _index_buffer     (QOpenGLBuffer::IndexBuffer),
        _vertex_attr      (-1),
        _mvp_attr         (-1),
        _color_attr       (-1),
        _number_of_points (0)
    {
      //
      // Init shaders
      //
      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Vertex, ":/gl/PinVertexShader.glsl"))
        throw Exception (QObject::tr ("Unable to initialize vertex shader: %1")
                         .arg (_shader.log ()));

      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Fragment, ":/gl/PinFragmentShader.glsl"))
        throw Exception (QObject::tr ("Unable to initialize fragment shader: %1")
                         .arg (_shader.log ()));

      if (!_shader.link ())
        throw Exception (QObject::tr ("Shader linking failed: %1")
                         .arg (_shader.log ()));

      //
      // Init vertex data
      //

      _vertex_attr = _shader.attributeLocation ("in_vertex");
      Q_ASSERT (_vertex_attr >= 0);

      _mvp_attr = _shader.uniformLocation ("in_mvp");
      Q_ASSERT (_mvp_attr >= 0);

      _color_attr = _shader.uniformLocation ("in_color");
      Q_ASSERT (_color_attr >= 0);

      QVector<PinData> data;
      QVector<GLushort> indices;

      static const int lats = 8;
      static const int longs = 8;

      for (int i=0; i <= lats; ++i)
        {
          double lat0 = M_PI * (-0.5 + static_cast<double> (i - 1) / lats);
          double z0  = sin (lat0);
          double zr0 =  cos (lat0);

          double lat1 = M_PI * (-0.5 + static_cast<double> (i) / lats);
          double z1 = sin (lat1);
          double zr1 = cos (lat1);

          for (int j=0; j <= longs; ++j)
            {
              double lng = 2 * M_PI * static_cast<double> (j - 1) / longs;
              double x = cos (lng);
              double y = sin (lng);

              data.push_back (PinData (QVector3D (x * zr0, y * zr0, z0)));
              data.push_back (PinData (QVector3D (x * zr1, y * zr1, z1)));
            }
        }

      for (int i=0; i < data.size (); ++i)
        indices.push_back (i);

      _number_of_points = data.size ();

      _vertex_buffer.create ();
      _vertex_buffer.bind ();
      _vertex_buffer.allocate (data.constData (), data.size () * sizeof (PinData));
      _vertex_buffer.release ();

      _index_buffer.create ();
      _index_buffer.bind ();
      _index_buffer.allocate (indices.constData (), indices.size () * sizeof (GLushort));
      _index_buffer.release ();
    }


    /*! Destructor */
    PinImpl::~PinImpl ()
    {
      _index_buffer.destroy ();
      _vertex_buffer.destroy ();
    }

    /*!
     * Draw pin
     *
     * @param pos   Drawing position
     * @param color Pin color
     */
    void PinImpl::draw (const QMatrix4x4& mvp,
                           const QVector3D& position,
                           const QColor& color,
                           double radius)
    {
      QOpenGLFunctions gl (QOpenGLContext::currentContext ());

      QMatrix4x4 p;
      p.translate (position);
      p.scale (radius);

      _vertex_buffer.bind ();
      _index_buffer.bind ();

      _shader.bind ();
      _shader.setUniformValue (_mvp_attr, mvp * p);
      _shader.setUniformValue (_color_attr, QVector3D (color.redF (), color.greenF (), color.blueF ()));

      int offset = 0;

      _shader.enableAttributeArray (_vertex_attr);
      _shader.setAttributeBuffer (_vertex_attr, GL_FLOAT, offset, 3, sizeof (PinData));

      offset += sizeof (QVector3D);

      gl.glDrawElements (GL_QUAD_STRIP, _number_of_points, GL_UNSIGNED_SHORT, 0);

      _shader.disableAttributeArray (_vertex_attr);

      _shader.release ();

      _index_buffer.release ();
      _vertex_buffer.release ();
    }


    //#**********************************************************************
    // CLASS HIP::GL::Pin
    //#**********************************************************************

    /*! Construction */
    Pin::Pin ()
      : _data (new PinImpl ())
    {
    }

    /*! Destructor */
    Pin::~Pin ()
    {
    }

    /*!
     * Draw pin
     *
     * @param pos   Drawing position
     * @param color Pin color
     */
    void Pin::draw (const QMatrix4x4& mvp,
                       const QVector3D& position,
                       const QColor& color,
                       double radius)
    {
      _data->draw (mvp, position, color, radius);
    }

  }
}

