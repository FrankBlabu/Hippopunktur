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

        int getVertexIndex () const  { return _vertex_index; }
        int getNormalIndex () const  { return _normal_index; }
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
      Material ();
      ~Material ();

      const QString& getName () const       { return _name; }
      const QVector3D& getAmbient () const  { return _ambient; }
      const QVector3D& getDiffuse () const  { return _diffuse; }
      const QVector3D& getSpecular () const { return _specular; }

      void setName (const QString& name)           { _name = name; }
      void setAmbient (const QVector3D& ambient)   { _ambient = ambient; }
      void setDiffuse (const QVector3D& diffuse)   { _diffuse = diffuse; }
      void setSpecular (const QVector3D& specular) { _specular = specular; }

    private:
      QString _name;
      QVector3D _ambient;
      QVector3D _diffuse;
      QVector3D _specular;
    };


    /*!
     * Class for loading and keeping a GL model
     */
    class Model
    {
    public:
      class Group
      {
      public:
        Group ();
        ~Group ();

        const QString& getName () const      { return _name; }
        const QString& getMaterial () const  { return _material; }
        const QList<Face>& getFaces () const { return _faces; }

        void setName (const QString& name)         { _name = name; }
        void setMaterial (const QString& material) { _material = material; }
        void addFace (const Face& face)            { _faces.push_back (face); }

      private:
        QString _name;
        QString _material;
        QList<Face> _faces;
      };

    public:
      Model (const QString& path); // throws Exception
      ~Model ();

      const QString& getName () const                { return _name; }
      const QVector<QVector3D>& getVertices () const { return _vertices; }
      const QVector<QVector3D>& getNormals () const  { return _normals; }
      const QVector<QVector2D>& getTextures () const { return _textures; }
      const QVector<Group>& getGroups () const       { return _groups; }

    private:
      void loadMaterial (const QString& path); // throws Exception
      Face::Point toPoint (const QString& t) const; // throws Exception

    private:
      QString _name;
      QVector<QVector3D> _vertices;
      QVector<QVector3D> _normals;
      QVector<QVector2D> _textures;
      QVector<Group> _groups;
      QVector<Material> _materials;
    };

  }
}

#endif
