//
//  QtSLiMGraphView_MutationFrequencySpectra.h
//  SLiM
//
//  Created by Ben Haller on 3/27/2020.
//  Copyright (c) 2020 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/slim/
//

//	This file is part of SLiM.
//
//	SLiM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//	SLiM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with SLiM.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QTSLIMGRAPHVIEW_MUTATIONFREQUENCYSPECTRA_H
#define QTSLIMGRAPHVIEW_MUTATIONFREQUENCYSPECTRA_H

#include <QWidget>

#include "QtSLiMGraphView.h"


class QtSLiMGraphView_MutationFrequencySpectra : public QtSLiMGraphView
{
    Q_OBJECT
    
public:
    explicit QtSLiMGraphView_MutationFrequencySpectra(QWidget *parent = nullptr);
    ~QtSLiMGraphView_MutationFrequencySpectra() override;
    
    QString graphTitle(void) override;
    void drawGraph(QPainter &painter, QRect interiorRect, QtSLiMWindow *controller) override;
    QtSLiMLegendSpec legendKey(void) override;
    bool providesStringForData(QtSLiMWindow *controller) override;
    QString stringForData(QtSLiMWindow *controller) override;
    
public slots:
    void controllerSelectionChanged(void) override;
    
private:
    double *mutationFrequencySpectrum(QtSLiMWindow *controller, int mutationTypeCount);    
};


#endif // QTSLIMGRAPHVIEW_MUTATIONFREQUENCYSPECTRA_H




































