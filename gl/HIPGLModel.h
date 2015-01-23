/*
 * HIPGLModel.h - Class for loading and keeping a GL model
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGLModel_h__
#define __HIPGLModel_h__

#include <QList>
#include <QString>
#include <QVector2D>
#include <QVector3D>


namespace HIP {
  namespace GL {

    /*!
     * Single face definition
     */
    class Face
    {
    public:
      class Point
      {
      public:
        Point (int vertex_index, int normal_index, int texture_index);

        int getVertexIndex () const { return _vertex_index; }
        int getNormalIndex () const { return _normal_index; }
        int getTextureIndex () const { return _texture_index; }

      private:
        int _vertex_index;
        int _normal_index;
        int _texture_index;
      };

    public:
      Face (const QList<Point>& points);
      ~Face ();

      const QList<Point>& getPoints () const { return _points; }

    private:
      QList<Point> _points;
    };


    /*!
     * Class for loading and keeping a GL model
     */
    class Model
    {
    public:
      Model (const QString& path); // throws Exception
      ~Model ();

      const QString& getName () const { return _name; }
      const QList<QVector3D>& getVertices () const { return _vertices; }
      const QList<QVector3D>& getNormals () const { return _normals; }
      const QList<QVector2D>& getTextures () const { return _textures; }
      const QList<Face>& getFaces () const { return _faces; }

    private:
      Face::Point toPoint (const QString& t) const; // throws Exception
      QVector3D toVector3d (const QString& x, const QString& y, const QString& z) const; // throws Exception
      QVector2D toVector2d (const QString& x, const QString& y) const; // throws Exception
      double toDouble (const QString& v) const; // throws Exception
      int toInt (const QString& v) const; // throws Exception

    private:
      QString _name;
      QList<QVector3D> _vertices;
      QList<QVector3D> _normals;
      QList<QVector2D> _textures;
      QList<Face> _faces;
    };

  }
}

#endif
