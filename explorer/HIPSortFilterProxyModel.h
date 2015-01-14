/*
 * HIPSortFilterProxyModel.h - Proxy model for sorting and filtering the explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPSortFilterProxyModel_h__
#define __HIPSortFilterProxyModel_h__

#include <QSortFilterProxyModel>
#include <QQmlParserStatus>
#include <QtQml/qjsvalue.h>

namespace HIP {
  namespace Explorer {

    /*
     * Proxy model for sorting and filtering the explorer
     */
    class SortFilterProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
    {
      Q_OBJECT
      Q_INTERFACES (QQmlParserStatus)

      Q_PROPERTY (int count READ count NOTIFY countChanged)
      Q_PROPERTY (QObject* source READ getSource WRITE setSource)

      Q_PROPERTY (QByteArray sortRole READ getSortRole WRITE setSortRole)
      Q_PROPERTY (Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder)

      Q_PROPERTY (QByteArray filterRole READ getFilterRole WRITE setFilterRole)
      Q_PROPERTY (QString filterString READ getFilterString WRITE setFilterString)
      Q_PROPERTY (FilterSyntax filterSyntax READ getFilterSyntax WRITE setFilterSyntax)

      Q_ENUMS (FilterSyntax)

    public:
      explicit SortFilterProxyModel (QObject* parent=0);

      QObject* getSource () const;
      void setSource (QObject* source);

      QByteArray getSortRole () const;
      void setSortRole (const QByteArray& role);

      void setSortOrder (Qt::SortOrder order);

      QByteArray getFilterRole () const;
      void setFilterRole (const QByteArray& role);

      QString getFilterString () const;
      void setFilterString (const QString& filter);

      enum FilterSyntax {
        RegExp,
        Wildcard,
        FixedString
      };

      FilterSyntax getFilterSyntax () const;
      void setFilterSyntax (FilterSyntax syntax);

      int count () const;
      Q_INVOKABLE QJSValue get (int index) const;

      void classBegin ();
      void componentComplete ();

    signals:
      void countChanged ();

    protected:
      int roleKey (const QByteArray &role) const;
      QHash<int, QByteArray> roleNames () const;
      bool filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const;

    private:
      bool _complete;
      QByteArray _sort_role;
      QByteArray _filter_role;
    };

  }
}

#endif
