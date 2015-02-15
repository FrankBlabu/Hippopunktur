/*
 * HIPGLData.h - Class for loading and keeping a GL model
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGLData_h__
#define __HIPGLData_h__

#include <QList>
#include <QMap>
#include <QString>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QSharedPointer>


namespace HIP {
  namespace GL {

    /*!
     * Single indexed point of a face
     */
    class Point
    {
    public:
      Point (int vertex_index, int normal_index, int texture_index);

      int getVertexIndex () const  { return _vertex_index; }
      int getNormalIndex () const  { return _normal_index; }
      int getTextureIndex () const { return _texture_index; }

      void setVertexIndex (int index) { _vertex_index = index; }
      void setNormalIndex (int index) { _normal_index = index; }
      void setTextureIndex (int index) { _texture_index = index; }

      bool operator< (const Point& point) const;

    private:
      int _vertex_index;
      int _normal_index;
      int _texture_index;
    };


    /*!
     * Single face definition
     */
    class Face
    {
    public:
      Face (const QList<Point>& points);
      ~Face ();

      const QList<Point>& getPoints () const { return _points; }

      void setNormalIndex (int index);

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
      float getDissolved () const           { return _dissolved; }
      float getSpecularExponent () const    { return _specular_exponent; }
      float getOpticalDensity () const      { return _optical_density; }
      const QString& getTexture () const    { return _texture; }

      void setName (const QString& name)           { _name = name; }
      void setAmbient (const QVector3D& ambient)   { _ambient = ambient; }
      void setDiffuse (const QVector3D& diffuse)   { _diffuse = diffuse; }
      void setSpecular (const QVector3D& specular) { _specular = specular; }
      void setDissolved (float dissolved)          { _dissolved = dissolved; }
      void setSpecularExponent (float ns)          { _specular_exponent = ns; }
      void setOpticalDensity (float ni)            { _optical_density = ni; }
      void setTexture (const QString& texture)     { _texture = texture; }

    private:
      QString _name;
      QVector3D _ambient;
      QVector3D _diffuse;
      QVector3D _specular;
      float _dissolved;
      float _specular_exponent;
      float _optical_density;
      QString _texture;
    };

    /*!
     * Single group
     */
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

      void setNormalIndex (int face_index, int index);

    private:
      QString _name;
      QString _material;
      QList<Face> _faces;
    };

    typedef QSharedPointer<Group> GroupPtr;

    /*!
     * Class for loading and keeping a GL model dataset
     */
    class Data
    {
    public:
      Data (const QString& path); // throws Exception
      ~Data ();

      const QString& getName () const                { return _name; }
      const QVector<QVector3D>& getVertices () const { return _vertices; }
      const QVector<QVector3D>& getNormals () const  { return _normals; }
      const QVector<QVector2D>& getTextures () const { return _textures; }
      const QVector<GroupPtr>& getGroups () const    { return _groups; }

      typedef QPair<QVector3D, QVector3D> Cube;
      const Cube& getBoundingBox () const { return _bounding_box; }
      const Material& getMaterial (const QString& name) const;

      void normalize ();
      void scale (double factor);

    private:
      void loadMaterial (const QString& path); // throws Exception
      void updateBoundingBox ();

      Point toPoint (const QString& t) const; // throws Exception

    private:
      QString _name;
      QVector<QVector3D> _vertices;
      QVector<QVector3D> _normals;
      QVector<QVector2D> _textures;
      QVector<GroupPtr> _groups;

      typedef QMap<QString, Material> MaterialMap;
      MaterialMap _materials;

      Cube _bounding_box;
    };

  }

  QDebug& operator<< (QDebug& stream, const GL::Face& face);
  QDebug& operator<< (QDebug& stream, const GL::Point& point);
  QDebug& operator<< (QDebug& stream, const GL::Material& material);
  QDebug& operator<< (QDebug& stream, const GL::Group& group);

}

#endif
