/*
 * HIPGLRenderable.h - Renderable model
 *
 * Frank Blankenburg, Feb. 2015
 */

#ifndef __HIPGLRenderable_h__
#define __HIPGLRenderable_h__

#include "gl/HIPGLData.h"
#include "database/HIPDatabase.h"

#include <QOpenGLBuffer>
#include <QMap>
#include <QMatrix4x4>
#include <QSet>
#include <QSharedPointer>

class QOpenGLTexture;
class QString;
class QVector3D;

namespace HIP {
  namespace GL {

    class Data;
    struct VertexCollector;

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
        int getElementSize () const;

      private:
        void addVertex (VertexCollector* collector, const Point& point) const;
        void setLightParameter (uint parameter, const QVector3D& value);

      private:
        const Data* _data;
        bool _has_texture;

        QOpenGLBuffer _vertex_buffer;
        QOpenGLBuffer _index_buffer;
        QMatrix4x4 _model_matrix;

        typedef QMap<QString, QOpenGLTexture*> TextureMap;
        TextureMap _textures;
    };

    typedef QSharedPointer<Renderable> RenderablePtr;


  }
}

#endif
