/*
 * HIPGLModel.h - Class for loading and keeping a GL model
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGLModel_h__
#define __HIPGLModel_h__

#include <QList>
#include <QMap>
#include <QString>
#include <QVector>
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
     * Class keeping material information
     */
    class Material
    {
    public:
      class Info
      {
      public:
        Info ();
        Info (const QString& name, const QVector3D& ambient, const QVector3D& diffuse, const QVector3D& specular);
        ~Info ();

        const QString& getName () const { return _name; }
        const QVector3D& getAmbient () const { return _ambient; }
        const QVector3D& getDiffuse () const { return _diffuse; }
        const QVector3D& getSpecular () const { return _specular; }

      private:
        QString _name;
        QVector3D _ambient;
        QVector3D _diffuse;
        QVector3D _specular;
      };

    public:
      Material ();
      ~Material ();

      bool exists (const QString& name) const;
      const Info& getInfo (const QString& name) const;

      void load (const QString& path); // throws Exception

    private:
      typedef QMap<QString, Info> InfoMap;
      InfoMap _infos;
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
      const QVector<QVector3D>& getVertices () const { return _vertices; }
      const QVector<QVector3D>& getNormals () const { return _normals; }
      const QVector<QVector2D>& getTextures () const { return _textures; }
      const QList<Face>& getFaces () const { return _faces; }

    private:
      Face::Point toPoint (const QString& t) const; // throws Exception

    private:
      QString _name;
      QVector<QVector3D> _vertices;
      QVector<QVector3D> _normals;
      QVector<QVector2D> _textures;
      QList<Face> _faces;
      Material _material;
    };

  }
}

#endif
