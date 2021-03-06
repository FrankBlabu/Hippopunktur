/*
 * hip_gl_renderable.cpp - Renderable model
 *
 * Frank Blankenburg, Feb. 2015
 */

#include "HIPGLRenderable.h"
#include "HIPGLData.h"
#include "ui_hip_gl_view.h"

#include "core/HIPException.h"
#include "core/HIPTools.h"

#include <QActionGroup>
#include <QKeyEvent>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOpenGLTexture>


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

    struct VertexCollector
    {
      QVector<VertexData> _vertex_data;
      QVector<GLuint> _index_data;

      typedef QMap<Point, int> PointIndexMap;
      PointIndexMap _point_indices;
    };


    //#**********************************************************************
    // CLASS HIP::GL::RenderableParameters
    //#**********************************************************************

    /* Constructor */
    RenderableParameters::RenderableParameters ()
      : _position       (0, 0, 0),
        _visible_groups (),
        _transparent    ()
    {
    }

    const QVector3D& RenderableParameters::getPosition () const
    {
      return _position;
    }

    void RenderableParameters::setPosition (const QVector3D& position)
    {
      _position = position;
    }

    const QSet<QString>& RenderableParameters::getVisibleGroups () const
    {
      return _visible_groups;
    }

    void RenderableParameters::setVisibleGroups (const QSet<QString>& visible_groups)
    {
      _visible_groups = visible_groups;
    }

    bool RenderableParameters::getTransparent () const
    {
      return _transparent;
    }

    void RenderableParameters::setTransparent (bool transparent)
    {
      _transparent = transparent;
    }


    //#**********************************************************************
    // CLASS HIP::GL::Renderable
    //#**********************************************************************

    /*! Constructor */
    Renderable::Renderable (const Data* data)
      : _data          (data),
        _has_texture   (false),
        _vertex_buffer (QOpenGLBuffer::VertexBuffer),
        _index_buffer  (QOpenGLBuffer::IndexBuffer),
        _model_matrix  (),
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
    void Renderable::paint (const QMatrix4x4& mvp, const RenderableParameters& parameters)
    {
      Q_UNUSED (mvp);

      QOpenGLFunctions gl (QOpenGLContext::currentContext ());

      int point_offset = 0;
      foreach (const GroupPtr& group, _data->getGroups ())
        {
          if (parameters.getVisibleGroups ().isEmpty () || parameters.getVisibleGroups ().contains (group->getName ()))
            {
              QOpenGLTexture* texture = 0;
              if (!group->getMaterial ().isEmpty ())
                {
                  TextureMap::const_iterator pos = _textures.find (group->getMaterial ());
                  if (pos != _textures.end ())
                    texture = pos.value ();

#if 0
                  const Material& material = _data->getMaterial (group->getMaterial ());

                  setLightParameter (GL_AMBIENT, material.getAmbient ());
                  setLightParameter (GL_DIFFUSE, material.getDiffuse ());
                  setLightParameter (GL_SPECULAR, material.getSpecular ());
                  setLightParameter (GL_POSITION, mvp * QVector3D (0, 0, 100));
                  gl.glLightf (GL_LIGHT0, GL_SPOT_EXPONENT, material.getSpecularExponent ());
                  gl.glEnable (GL_LIGHT0);
#endif
                }

              if (texture != 0)
                texture->bind ();

              gl.glDrawElements (GL_TRIANGLES, group->getFaces ().size () * 3, GL_UNSIGNED_INT, (void*)(point_offset * sizeof (GLuint)));

              if (texture != 0)
                texture->release ();
            }

          point_offset += group->getFaces ().size () * 3;
        }
    }

    /*
     * Return size of a single rendered element
     */
    int Renderable::getElementSize () const
    {
      return sizeof (VertexData);
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
      Q_UNUSED (parameter);
      Q_UNUSED (value);
#if 0
      float array[] = {value.x (), value.y (), value.z (), 1.0f};
      glLightfv (GL_LIGHT0, parameter, array);
#endif
    }


  }
}
