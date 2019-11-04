#include "mpbrushlib.h"

MPBrushLib::MPBrushLib(const QString &brushLibPath)
{

}


void MPBrushLib::selectBrush (QString brushName)
{
//    if (!isValid()) return;
//    QListWidget*      p_page = NULL;
//    QListWidgetItem * p_item = NULL;
//    // We search for the brush requested :
//    for (int page = count()-1 ; page >= 0 && !p_item ; page--)
//    {
//        // reverse loop so we leave it with first page
//        p_page = dynamic_cast<QListWidget*>(widget(page));
//        QString caption = tabText(page);
//        const QStringList subList = m_brushLib.value(caption);
//        if (!brushName.isEmpty()) for (int idx = 0 ; idx < subList.count() ; idx++)
//        {
//            if (subList.at(idx) == brushName) { p_item = p_page->item(idx); break; }
//        }
//    }
//    // default one : we use the first tab page & the first item available:
//    if (!p_item && p_page && p_page->count()) p_item = p_page->item(0);
//    // Update GUI + load the brush (if any)
//    if (p_item)
//    {
//        setCurrentWidget(p_page);
//        p_page->setCurrentItem (p_item);
//        itemClicked(p_item);
//    }
}
