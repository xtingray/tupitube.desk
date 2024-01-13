/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "tupsceneselector.h"

#include <QListWidget>
#include <QListWidgetItem>

TupSceneSelector::TupSceneSelector() : TupExportWizardPage(tr("Select Scenes"))
{
    setTag("SCENE");
    m_selector = new TItemSelector(tr("Add Scene"), tr("Remove Scene"));

    connect(m_selector, SIGNAL(changed()), this, SLOT(updateState()));

    setWidget(m_selector);
}

TupSceneSelector::~TupSceneSelector()
{
}

bool TupSceneSelector::isComplete() const
{
    return m_selector->selectedItems().count() > 0;
}

void TupSceneSelector::resetUI()
{
}

void TupSceneSelector::setFormatType(ExportOutputFormat exportFormat)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSceneSelector::setFormatType()]";
    #endif

    format = exportFormat;
    bool visible = false;
    if (format == Animation)
        visible = true;

    setDurationLabelVisible(visible);
}

void TupSceneSelector::setScenes(const QList<TupScene *> &scenesList, int fps)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSceneSelector::setScenes()]";
    #endif

    scenes = scenesList;
    m_selector->clear();
    duration = 0;

    if (scenes.count() > 1) {
        int pos = 1;
        foreach (TupScene *scene, scenes) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupSceneSelector::setScenes()] - Adding scene ->" << scene->getSceneName();
            #endif

            sceneDuration << static_cast<double>(scene->framesCount()) / static_cast<double>(fps);
            m_selector->addItem(QString("%1: ").arg(pos) + scene->getSceneName());
            pos++;
        }

        m_selector->selectFirstItem();
        duration = static_cast<double>(scenes.first()->framesCount()) / static_cast<double>(fps);

        #ifdef TUP_DEBUG
            qDebug() << "[TupSceneSelector::setScenes()] - Available scenes ->" << (pos - 1);
            qDebug() << "[TupSceneSelector::setScenes()] - Scene duration ->" << duration;
        #endif
    } else {
        TupScene *scene = scenes.first();
        m_selector->addItem(QString("1: ") + scene->getSceneName());
        m_selector->addSelectedItem(QString("1: ") + scene->getSceneName());

        duration = static_cast<double>(scene->framesCount()) / static_cast<double>(fps);
        sceneDuration << duration;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupSceneSelector::setScenes()] - Scene duration ->" << duration;
    #endif

    m_selector->updateDurationLabel(QString::number(duration));
}

void TupSceneSelector::aboutToNextPage()
{
    emit selectedScenes(m_selector->selectedIndexes());
}

void TupSceneSelector::updateState()
{
    QList<int> indexes = m_selector->selectedIndexes();

    #ifdef TUP_DEBUG
        qDebug() << "[TupSceneSelector::updateState()] - selected indexes ->" << indexes;
    #endif

    if (format == Animation) {
        duration = 0;
        foreach(int index, indexes)
            duration += sceneDuration.at(index);

        m_selector->updateDurationLabel(QString::number(duration));
    }

    emit completed();
}

void TupSceneSelector::updateScenesList()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSceneSelector::updateScenesList()] - duration ->" << duration;
    #endif
}

void TupSceneSelector::setDurationLabelVisible(bool visible)
{
    m_selector->setDurationLabelVisible(visible);
}
