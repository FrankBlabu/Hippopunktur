/*
 * hip_gl_model.cpp - Class for loading and keeping a GL model
 *
 * Frank Blankenburg, Jan. 2014
 */

#include "HIPGLModel.h"
#include "core/HIPException.h"
#include "core/HIPTools.h"

#include <QDebug>
#include <QTextStream>

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // Local functions
    //#**********************************************************************

    namespace {

      /* Convert string into double value */
      qreal toReal (const QString& v) // throws Exception
      {
        bool ok = false;
        qreal d = v.toFloat (&ok);

        if (!ok)
          throw Exception (QObject::tr ("Double value expected"));

        return d;
      }

      /* Convert string into integer value */
      int toInt (const QString& v) // throws Exception
      {
        bool ok = false;
        int i = v.toInt (&ok);

        if (!ok)
          throw Exception (QObject::tr ("Double value expected"));

        return i;
      }

      /* Convert string tuple into 3d vector */
      QVector3D toVector3d (const QString& x, const QString& y, const QString& z) // throws Exception
      {
        return QVector3D (toReal (x), toReal (y), toReal (z));
      }

      /* Convert string tuple into 2d vector */
      QVector2D toVector2d (const QString& x, const QString& y) // throws Exception
      {
        return QVector2D (toReal (x), toReal (y));
      }

    }


    //#**********************************************************************
    // CLASS HIP::GL::Face::Point
    //#**********************************************************************

    /*! Constructor */
    Face::Point::Point (int vertex_index, int normal_index, int texture_index)
      : _vertex_index  (vertex_index),
        _normal_index  (normal_index),
        _texture_index (texture_index)
    {
    }


    //#**********************************************************************
    // CLASS HIP::GL::Face
    //#**********************************************************************

    /*! Constructor */
    Face::Face (const QList<Point>& points)
      : _points (points)
    {
    }

    /*! Destructor */
    Face::~Face ()
    {
    }

    //#**********************************************************************
    // CLASS HIP::GL::Material
    //#**********************************************************************

    /*! Constructor */
    Material::Material ()
      : _name     (""),
        _ambient  (),
        _diffuse  (),
        _specular ()
    {
    }

    /*! Destructor */
    Material::~Material ()
    {
    }


    //#**********************************************************************
    // CLASS HIP::GL::Model::Group
    //#**********************************************************************

    /*! Constructor */
    Model::Group::Group ()
      : _name     (),
        _material (),
        _faces    ()
    {
    }

    /*! Destructor */
    Model::Group::~Group ()
    {
    }


    //#**********************************************************************
    // CLASS HIP::GL::Model
    //#**********************************************************************

    /*! Constructor */
    Model::Model (const QString& path)
      : _name (),
        _vertices (),
        _normals  (),
        _textures ()
    {
      QString content = Tools::loadResource<QString> (path);
      QString material_library;

      QTextStream file (&content, QIODevice::ReadOnly);

      Group group;

      for (QString line=file.readLine ().trimmed (); !line.isNull (); line=file.readLine ().trimmed ())
        {
          QTextStream in (&line, QIODevice::ReadOnly);

          QString tag;
          in >> tag;
          tag = tag.toLower ();

          //
          // Object name
          //
          if (tag == "o")
            {
              in >> _name;
            }

          //
          // Vertex
          //
          else if (tag == "v")
            {
              QString x, y, z;
              in >> x >> y >> z;
              _vertices.push_back (toVector3d (x, y, z));
            }

          //
          // Vertex normal
          //
          else if (tag == "vn")
            {
              QString x, y, z;
              in >> x >> y >> z;
              _normals.push_back (toVector3d (x, y, z));
            }

          //
          // Vertex texture
          //
          else if (tag == "vt")
            {
              QString x, y;
              in >> x >> y;
              _textures.push_back (toVector2d (x, y));
            }

          //
          // Face
          //
          else if (tag == "f")
            {
              QList<Face::Point> points;

              for (in >> tag; !tag.isNull (); in >> tag)
                points.push_back (toPoint (tag));

             group.addFace (Face (points));
            }

          //
          // Group
          //
          else if (tag == "g")
            {
              QString name;
              in >> name;

              if (!group.getFaces ().isEmpty ())
                _groups.push_back (group);

              group = Group ();
              group.setName (name);
            }

          //
          // Used material
          //
          else if (tag == "usemtl")
            {
              QString material;
              in >> material;
              group.setMaterial (material);
            }

          //
          // Related material library
          //
          else if (tag == "mtllib")
            {
              in >> material_library;
            }
        }

      _groups.push_back (group);

      //
      // Sanity check
      //
      Q_ASSERT (!_vertices.isEmpty ());
      Q_ASSERT (!_normals.isEmpty ());
      Q_ASSERT (!_textures.isEmpty ());

      foreach (const Model::Group& group, _groups)
        {
          foreach (const Face& face, group.getFaces ())
            {
              Q_ASSERT (!face.getPoints ().isEmpty ());

              foreach (const Face::Point& point, face.getPoints ())
                {
                  Q_UNUSED (point);
                  Q_ASSERT (point.getVertexIndex () >= -1 &&
                            point.getVertexIndex () < _vertices.size ());
                  Q_ASSERT (point.getNormalIndex () >= -1 &&
                            point.getNormalIndex () < _normals.size ());
                  Q_ASSERT (point.getTextureIndex () >= -1 &&
                            point.getTextureIndex () < _textures.size ());
                }
            }
        }

      //
      // Load material
      //
      if (!material_library.isEmpty ())
        {
          QStringList p = path.split ('/');
          Q_ASSERT (!p.isEmpty ());
          p[p.size () - 1] = material_library;

          loadMaterial (p.join ('/'));
        }
    }

    /*! Load material library */
    void Model::loadMaterial (const QString& path)
    {
      QString content = Tools::loadResource<QString> (path);
      QTextStream file (&content, QIODevice::ReadOnly);

      Material material;

      for (QString line=file.readLine ().trimmed (); !line.isNull (); line=file.readLine ().trimmed ())
        {
          QTextStream in (&line, QIODevice::ReadOnly);

          QString tag;
          in >> tag;
          tag = tag.toLower ();

          //
          // Related material library
          //
          if (tag == "newmtl")
            {
              if (!material.getName ().isEmpty ())
                _materials.push_back (material);
              material = Material ();

              QString name;
              in >> name;
              material.setName (name);
            }

          //
          // Ambient
          //
          else if (tag == "Ka")
            {
              QString x, y, z;
              in >> x >> y >> z;
              material.setAmbient (toVector3d (x, y, z));
            }

          //
          // Diffuse
          //
          else if (tag == "Kd")
            {
              QString x, y, z;
              in >> x >> y >> z;
              material.setDiffuse (toVector3d (x, y, z));
            }

          //
          // Specular
          //
          else if (tag == "Ks")
            {
              QString x, y, z;
              in >> x >> y >> z;
              material.setSpecular (toVector3d (x, y, z));
            }
        }

      if (!material.getName ().isEmpty ())
        _materials.push_back (material);
    }


    /*! Destructor */
    Model::~Model ()
    {
    }

    /* Convert string into a face point */
    Face::Point Model::toPoint (const QString& t) const // throws Exception
    {
      QStringList parts = t.split ('/', QString::KeepEmptyParts);
      if (parts.isEmpty ())
        throw Exception (QObject::tr ("Invalid face definition"));

      int vertex_index = -1;
      if (parts.size () > 0 && !parts[0].isEmpty ())
        vertex_index = toInt (parts[0]) - 1;

      int texture_index = -1;
      if (parts.size () > 1 && !parts[1].isEmpty ())
        texture_index = toInt (parts[1]) - 1;

      int normal_index = -1;
      if (parts.size () > 2 && !parts[2].isEmpty ())
        normal_index = toInt (parts[2]) - 1;

      return Face::Point (vertex_index, normal_index, texture_index);
    }

  }
}
