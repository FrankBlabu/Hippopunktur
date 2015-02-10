/*
 * hip_gl_painter.cpp - GL based sphere
 *
 * Frank Blankenburg, Feb. 2015
 */

#include "HIPGLSphere.h"
#include "core/HIPException.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // CLASS HIP::GL::SphereData
    //#**********************************************************************

    namespace {

      struct SphereData
      {
        SphereData () {}
        SphereData (const QVector3D& vertex, const QVector3D& normal)
          : _vertex (vertex), _normal (normal) {}

        QVector3D _vertex;
        QVector3D _normal;
      };

    }


    //#**********************************************************************
    // CLASS HIP::GL::SphereImpl
    //#**********************************************************************

    /*!
     * Sphere data keeping class
     */
    class SphereImpl
    {
    public:
      SphereImpl (double radius);
      ~SphereImpl ();

      void draw (const QMatrix4x4& mvp,
                 const QVector3D& position,
                 const QColor& color);

    private:
      QOpenGLShaderProgram _shader;
      QOpenGLBuffer _vertex_buffer;
      QOpenGLBuffer _index_buffer;

      int _vertex_attr;
      int _normal_attr;
      int _mvp_attr;
      int _color_attr;

      int _number_of_triangles;
    };

    /*! Constructor */
    SphereImpl::SphereImpl (double radius)
      : _shader              (),
        _vertex_buffer       (QOpenGLBuffer::VertexBuffer),
        _index_buffer        (QOpenGLBuffer::IndexBuffer),
        _vertex_attr         (-1),
        _normal_attr         (-1),
        _mvp_attr            (-1),
        _color_attr          (-1),
        _number_of_triangles (0)
    {
      Q_UNUSED (radius);

      //
      // Init shaders
      //
      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Vertex, ":/gl/SphereVertexShader.glsl"))
        throw Exception (QObject::tr ("Unable to initialize vertex shader: %1")
                         .arg (_shader.log ()));

      if (!_shader.addShaderFromSourceFile (QOpenGLShader::Fragment, ":/gl/SphereFragmentShader.glsl"))
        throw Exception (QObject::tr ("Unable to initialize fragment shader: %1")
                         .arg (_shader.log ()));

      if (!_shader.link ())
        throw Exception (QObject::tr ("Shader linking failed: %1")
                         .arg (_shader.log ()));

      //
      // Init vertex data
      //

      _vertex_attr = _shader.attributeLocation ("in_vertex");
      _normal_attr = _shader.attributeLocation ("in_normal");
      _mvp_attr = _shader.attributeLocation ("in_mvp");
      _color_attr = _shader.attributeLocation ("in_color");

      QVector<SphereData> data;
      QVector<GLushort> indices;

#if 0
      static const int lats = 16;
      static const int longs = 16;

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

              data.push_back (SphereData (QVector3D (x * zr0, y * zr0, z0) * radius,
                                          QVector3D (x * zr0, y * zr0, z0)));
              data.push_back (SphereData (QVector3D (x * zr1, y * zr1, z1) * radius,
                                          QVector3D (x * zr1, y * zr1, z1)));
            }
        }

      for (int i=2; i < data.size (); i += 2)
        {
          indices.push_back (i - 2);
          indices.push_back (i - 1);
          indices.push_back (i);

          indices.push_back (i - 2);
          indices.push_back (i);
          indices.push_back (i + 1);

          _number_of_triangles += 2;
        }
#else
      data.push_back (SphereData (QVector3D (-0.5, +0.0, 0.0), QVector3D (0.0, 0.0, +1.0)));
      data.push_back (SphereData (QVector3D (+0.5, +0.0, 0.0), QVector3D (0.0, 0.0, +1.0)));
      data.push_back (SphereData (QVector3D (0.0, 0.5, 0.0), QVector3D (0.0, 0.0, +1.0)));

      indices.push_back (0);
      indices.push_back (1);
      indices.push_back (2);

      _number_of_triangles = 1;
#endif

      _vertex_buffer.create ();
      _vertex_buffer.bind ();
      _vertex_buffer.allocate (data.constData (), data.size () * sizeof (SphereData));

      _index_buffer.create ();
      _index_buffer.bind ();
      _index_buffer.allocate (indices.constData (), indices.size () * sizeof (GLushort));
    }


    /*! Destructor */
    SphereImpl::~SphereImpl ()
    {
      _index_buffer.destroy ();
      _vertex_buffer.destroy ();
    }

    /*!
     * Draw sphere
     *
     * @param pos   Drawing position
     * @param color Sphere color
     */
    void SphereImpl::draw (const QMatrix4x4& mvp,
                           const QVector3D& position,
                           const QColor& color)
    {
      Q_UNUSED (position);

      QOpenGLFunctions gl (QOpenGLContext::currentContext ());

      _shader.bind ();
      _shader.setUniformValue (_mvp_attr, mvp);
      _shader.setUniformValue (_color_attr, QVector3D (color.redF (), color.greenF (), color.blueF ()));

      int offset = 0;

      _shader.enableAttributeArray (_vertex_attr);
      _shader.setAttributeBuffer (_vertex_attr, GL_FLOAT, offset, 3, sizeof (SphereData));

      offset += sizeof (QVector3D);

      _shader.enableAttributeArray (_normal_attr);
      _shader.setAttributeBuffer (_normal_attr, GL_FLOAT, offset, 3, sizeof (SphereData));

      offset += sizeof (QVector3D);

      gl.glDrawElements (GL_TRIANGLES, _number_of_triangles * 3, GL_UNSIGNED_SHORT, 0);

      _shader.disableAttributeArray (_normal_attr);
      _shader.disableAttributeArray (_vertex_attr);

      _shader.release ();
    }


    //#**********************************************************************
    // CLASS HIP::GL::Sphere
    //#**********************************************************************

    /*! Construction */
    Sphere::Sphere (double radius)
      : _data (new SphereImpl (radius))
    {
    }

    /*! Destructor */
    Sphere::~Sphere ()
    {
    }

    /*!
     * Draw sphere
     *
     * @param pos   Drawing position
     * @param color Sphere color
     */
    void Sphere::draw (const QMatrix4x4& mvp,
                       const QVector3D& position,
                       const QColor& color)
    {
      _data->draw (mvp, position, color);
    }

  }
}

