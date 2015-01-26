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
    // CLASS HIP::GL::Model
    //#**********************************************************************

    /*! Constructor */
    Model::Model (const QString& path)
      : _name     (),
        _vertices (),
        _normals  (),
        _textures (),
        _faces    ()
    {
      QString content = Tools::loadResource<QString> (path);
      QString material_library;

      QTextStream file (&content, QIODevice::ReadOnly);

      for (QString line=file.readLine ().trimmed (); !line.isNull (); line=file.readLine ().trimmed ())
        {
          QTextStream in (&line, QIODevice::ReadOnly);

          QString tag;
          in >> tag;
          tag = tag.toLower ();

          //
          // Related material library
          //
          if (tag == "mtllib")
            {
              in >> material_library;
            }

          //
          // Object name
          //
          else if (tag == "o")
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

              _faces.push_back (Face (points));
            }
        }

#if 0
      qDebug () << "* Load model, path=" << path;
      qDebug () << "  name=" << _name;
      qDebug () << "  " << _vertices.size () << " vertices";
      qDebug () << "  " << _normals.size () << " normals";
      qDebug () << "  " << _textures.size () << "  textures";
      qDebug () << "  " << _faces.size () << " faces";
#endif

      //
      // Sanity check
      //
      Q_ASSERT (!_vertices.isEmpty ());
      Q_ASSERT (!_normals.isEmpty ());
      Q_ASSERT (!_textures.isEmpty ());
      Q_ASSERT (!_faces.isEmpty ());

      foreach (const Face& face, _faces)
        {
          Q_ASSERT (!face.getPoints ().isEmpty ());

          foreach (const Face::Point& point, face.getPoints ())
            {
              Q_UNUSED (point);
              Q_ASSERT (point.getVertexIndex () >= -1 &&
                        point.getVertexIndex () <= _vertices.size ());
              Q_ASSERT (point.getNormalIndex () >= -1 &&
                        point.getNormalIndex () <= _normals.size ());
              Q_ASSERT (point.getTextureIndex () >= -1 &&
                        point.getTextureIndex () <= _textures.size ());
            }
        }
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
        vertex_index = toDouble (parts[0]);

      int texture_index = -1;
      if (parts.size () > 1 && !parts[1].isEmpty ())
        texture_index = toDouble (parts[1]);

      int normal_index = -1;
      if (parts.size () > 2 && !parts[2].isEmpty ())
        normal_index = toDouble (parts[2]);

      return Face::Point (vertex_index, normal_index, texture_index);
    }

    /* Convert string tuple into 3d vector */
    QVector3D Model::toVector3d (const QString& x, const QString& y, const QString& z) const // throws Exception
    {
      return QVector3D (toDouble (x), toDouble (y), toDouble (z));
    }

    /* Convert string tuple into 2d vector */
    QVector2D Model::toVector2d (const QString& x, const QString& y) const // throws Exception
    {
      return QVector2D (toDouble (x), toDouble (y));
    }

    /* Convert string into double value */
    double Model::toDouble (const QString& v) const // throws Exception
    {
      bool ok = false;
      double d = v.toDouble (&ok);

      if (!ok)
        throw Exception (QObject::tr ("Double value expected"));

      return d;
    }

    /* Convert string into integer value */
    int Model::toInt (const QString& v) const // throws Exception
    {
      bool ok = false;
      int i = v.toInt (&ok);

      if (!ok)
        throw Exception (QObject::tr ("Double value expected"));

      return i;
    }

  }
}
