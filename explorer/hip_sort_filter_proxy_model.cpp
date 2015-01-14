/*
 * hip_sort_filter_proxy_model.cpp - Proxy model for sorting and filtering the explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPSortFilterProxyModel.h"

#include <QtDebug>
#include <QtQml>

namespace HIP {
  namespace Explorer {

    /*! Constructor */
    SortFilterProxyModel::SortFilterProxyModel (QObject *parent)
      : QSortFilterProxyModel (parent),
        _complete (false)
    {
      connect (this, SIGNAL (rowsInserted (QModelIndex, int, int)), this, SIGNAL (countChanged ()));
      connect (this, SIGNAL (rowsRemoved (QModelIndex, int, int)), this, SIGNAL (countChanged ()));
    }

    int SortFilterProxyModel::count () const
    {
      return rowCount ();
    }

    QObject* SortFilterProxyModel::getSource () const
    {
      return sourceModel ();
    }

    void SortFilterProxyModel::setSource (QObject* source)
    {
      setSourceModel (qobject_cast<QAbstractItemModel*> (source));
    }

    QByteArray SortFilterProxyModel::getSortRole () const
    {
      return _sort_role;
    }

    void SortFilterProxyModel::setSortRole (const QByteArray& role)
    {
      if (_sort_role != role)
        {
          _sort_role = role;
          if (_complete)
            QSortFilterProxyModel::setSortRole (roleKey (role));
        }
    }

    void SortFilterProxyModel::setSortOrder (Qt::SortOrder order)
    {
      QSortFilterProxyModel::sort (0, order);
    }

    QByteArray SortFilterProxyModel::getFilterRole () const
    {
      return _filter_role;
    }

    void SortFilterProxyModel::setFilterRole (const QByteArray& role)
    {
      if (_filter_role != role)
        {
          _filter_role = role;
          if (_complete)
            QSortFilterProxyModel::setFilterRole (roleKey (role));
        }
    }

    QString SortFilterProxyModel::getFilterString () const
    {
      return filterRegExp ().pattern  ();
    }

    void SortFilterProxyModel::setFilterString (const QString& filter)
    {
      setFilterRegExp (QRegExp (filter, filterCaseSensitivity (), static_cast<QRegExp::PatternSyntax> (getFilterSyntax ())));
    }

    SortFilterProxyModel::FilterSyntax SortFilterProxyModel::getFilterSyntax () const
    {
      return static_cast<FilterSyntax> (filterRegExp ().patternSyntax ());
    }

    void SortFilterProxyModel::setFilterSyntax (SortFilterProxyModel::FilterSyntax syntax)
    {
      setFilterRegExp (QRegExp (getFilterString (), filterCaseSensitivity (), static_cast<QRegExp::PatternSyntax> (syntax)));
    }

    QJSValue SortFilterProxyModel::get (int idx) const
    {
      QJSEngine* engine = qmlEngine (this);
      QJSValue value = engine->newObject ();
      if (idx >= 0 && idx < count  ())
        {
          QHash<int, QByteArray> roles = roleNames ();
          QHashIterator<int, QByteArray> it (roles);
          while (it.hasNext ())
            {
              it.next  ();
              value.setProperty (QString::fromUtf8 (it.value ()), data (index (idx, 0), it.key ()).toString  ());
            }
        }

      return value;
    }

    void SortFilterProxyModel::classBegin ()
    {
    }

    void SortFilterProxyModel::componentComplete ()
    {
      _complete = true;
      if (!_sort_role.isEmpty ())
        QSortFilterProxyModel::setSortRole (roleKey (_sort_role));
      if (!_filter_role.isEmpty ())
        QSortFilterProxyModel::setFilterRole (roleKey (_filter_role));
    }

    int SortFilterProxyModel::roleKey (const QByteArray& role) const
    {
      QHash<int, QByteArray> roles = roleNames ();
      QHashIterator<int, QByteArray> it (roles);
      while (it.hasNext ())
        {
          it.next ();
          if (it.value () == role)
            return it.key ();
        }

      return -1;
    }

    QHash<int, QByteArray> SortFilterProxyModel::roleNames () const
    {
      if (QAbstractItemModel* source = sourceModel ())
        return source->roleNames  ();

      return QHash<int, QByteArray> ();
    }

    bool SortFilterProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
    {
      QRegExp rx = filterRegExp ();
      if (rx.isEmpty ())
        return true;

      QAbstractItemModel* model = sourceModel ();
      if (getFilterRole ().isEmpty ())
        {
          QHash<int, QByteArray> roles = roleNames ();
          QHashIterator<int, QByteArray> it (roles);
          while (it.hasNext  ())
            {
              it.next ();
              QModelIndex sourceIndex = model->index (sourceRow, 0, sourceParent);
              QString key = model->data (sourceIndex, it.key ()).toString  ();
              if (key.contains (rx))
                return true;
            }
          return false;
        }

      QModelIndex sourceIndex = model->index (sourceRow, 0, sourceParent);
      if (!sourceIndex.isValid ())
        return true;

      QString key = model->data (sourceIndex, roleKey (getFilterRole ())).toString  ();
      return key.contains (rx);
    }

  }
}
