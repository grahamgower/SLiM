#ifndef QTSLIMHELPWINDOW_H
#define QTSLIMHELPWINDOW_H

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextDocumentFragment>
#include <QMap>
#include <QStyledItemDelegate>

#include "eidos_call_signature.h"

class QCloseEvent;
class EidosPropertySignature;
class EidosObjectClass;


// SLiMgui uses an NSDictionary-based design to hold the documentation tree data structure.  Here we
// instead leverage the fact that QTreeWidgetItem already represents a tree structure, and simply
// use it directly to represent the documentation tree.  We use a custom subclass so we can keep
// associated doc, which is held as a QTextDocumentFragment; this is used only by leaves

class QtSLiMHelpItem : public QTreeWidgetItem
{
    // no Q_OBJECT; QTreeWidgetItem is not a QObject subclass!
    
public:
    QTextDocumentFragment *doc_fragment = nullptr;
    
    QtSLiMHelpItem(QTreeWidget *parent) : QTreeWidgetItem(parent) {}
    QtSLiMHelpItem(QTreeWidgetItem *parent) : QTreeWidgetItem(parent) {}
    ~QtSLiMHelpItem() override;
};


// This subclass of QStyledItemDelegate provides custom drawing for the outline view.

class QtSLiMHelpOutlineDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    
public:
    QtSLiMHelpOutlineDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    ~QtSLiMHelpOutlineDelegate(void) override;
    
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};


// We keep a QMap of topics in the currently building hierarchy so we can find the right parent
// for each new item.  This is a temporary data structure used only during the build of each section.
typedef QMap<QString, QtSLiMHelpItem *> QtSLiMTopicMap;


// This class provides a singleton help window

namespace Ui {
class QtSLiMHelpWindow;
}

class QtSLiMHelpWindow : public QDialog
{
    Q_OBJECT
    
public:
    static QtSLiMHelpWindow &instance(void);
    
    void enterSearchForString(QString searchString, bool titlesOnly = true);
    
private:
    // singleton pattern
    explicit QtSLiMHelpWindow(QWidget *parent = nullptr);
    QtSLiMHelpWindow() = default;
    virtual ~QtSLiMHelpWindow() override;
    
    // Add topics and items drawn from a specially formatted HTML file, under a designated top-level heading.
    // The signatures found for functions, methods, and prototypes will be checked against the supplied lists,
    // if they are not NULL, and if matches are found, colorized versions will be substituted.
    void addTopicsFromRTFFile(const QString &htmlFile,
                              const QString &topLevelHeading,
                              const std::vector<EidosFunctionSignature_SP> *functionList,
                              const std::vector<const EidosMethodSignature*> *methodList,
                              const std::vector<const EidosPropertySignature*> *propertyList);
    
    const std::vector<const EidosPropertySignature*> *slimguiAllPropertySignatures(void);
    const std::vector<const EidosMethodSignature*> *slimguiAllMethodSignatures(void);
    
    // Searching
    bool findItemsMatchingSearchString(QTreeWidgetItem *root, const QString searchString, bool titlesOnly, std::vector<QTreeWidgetItem *> &matchKeys, std::vector<QTreeWidgetItem *> &expandItems);
    void searchFieldChanged(void);
    void searchScopeToggled(void);
    
    // Smart expand/contract, with the option key making it apply to all children as well
    void recursiveExpand(QTreeWidgetItem *item);
    void recursiveCollapse(QTreeWidgetItem *item);
    void itemClicked(QTreeWidgetItem *item, int column);
    void itemCollapsed(QTreeWidgetItem *item);
    void itemExpanded(QTreeWidgetItem *item);
    
    // Check for complete documentation
    QtSLiMHelpItem *findObjectWithKeySuffix(const QString searchKeySuffix, QTreeWidgetItem *searchItem);
    QtSLiMHelpItem *findObjectForKeyEqualTo(const QString searchKey, QTreeWidgetItem *searchItem);
    void checkDocumentationOfFunctions(const std::vector<EidosFunctionSignature_SP> *functions);
    void checkDocumentationOfClass(EidosObjectClass *classObject);
    
    // responding to events
    void closeEvent(QCloseEvent *e) override;
    void outlineSelectionChanged(void);
    
    // Internals
    QTreeWidgetItem *parentItemForSection(const QString &sectionString, QtSLiMTopicMap &topics, QtSLiMHelpItem *topItem);
    QtSLiMHelpItem *createItemForSection(const QString &sectionString, QString title, QtSLiMTopicMap &topics, QtSLiMHelpItem *topItem);
    
private:
    int searchType;		// 0 == Title, 1 == Content; equal to the tags on the search type menu items
    bool doingProgrammaticCollapseExpand = false;   // used to distinguish user actions from programmatic ones
    bool doingProgrammaticSelection = false;        // used to distinguish user actions from programmatic ones
    
    Ui::QtSLiMHelpWindow *ui;
};


#endif // QTSLIMHELPWINDOW_H






























