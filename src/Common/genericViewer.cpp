/**
 * This file is a part of Qtpfsgui package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2009 Franco Comida
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ----------------------------------------------------------------------
 *
 * @author Franco Comida <fcomida@users.sourceforge.net>
 */

#include <QMessageBox>
#include "genericViewer.h"

GenericViewer::GenericViewer(QWidget *parent, bool ns, bool ncf): QWidget(parent), NeedsSaving(ns), noCloseFlag(ncf), isSelectionReady(false) {
	
	panIconWidget=new PanIconWidget(this);

	VBL_L = new QVBoxLayout(this);
	VBL_L->setSpacing(0);
	VBL_L->setMargin(0);

	toolBar = new QToolBar(tr(""),this);
	VBL_L->addWidget(toolBar);

	scrollArea = new SmartScrollArea(this, imageLabel);
	scrollArea->setBackgroundRole(QPalette::Shadow);
	VBL_L->addWidget(scrollArea);

	cornerButton=new QToolButton(this);
	cornerButton->setToolTip("Pan the image to a region");
	cornerButton->setIcon(QIcon(":/new/prefix1/images/move.png"));
	scrollArea->setCornerWidget(cornerButton);
	connect(cornerButton, SIGNAL(pressed()), this, SLOT(slotCornerButtonPressed()));
	connect(scrollArea, SIGNAL(selectionReady(bool)), this, SIGNAL(selectionReady(bool)));
}

void GenericViewer::fitToWindow(bool checked) {
	scrollArea->fitToWindow(checked);
}

void GenericViewer::zoomIn() {
	scrollArea->zoomIn();
}

void GenericViewer::zoomOut() {
	scrollArea->zoomOut();
}

bool GenericViewer::getFittingWin() {
	return scrollArea->isFitting();
}

void GenericViewer::normalSize() {
	scrollArea->normalSize();
}

const float GenericViewer::getScaleFactor() {
	return scrollArea->getScaleFactor();
}

void GenericViewer::closeEvent ( QCloseEvent * event ) {
	if (NeedsSaving) {
		QMessageBox::StandardButton ret=QMessageBox::warning(this,tr("Unsaved changes..."),tr("This Hdr has unsaved changes.<br>Are you sure you want to close it?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
		if (ret==QMessageBox::Yes)
			event->accept();
		else
			event->ignore();
	} 
	else if (noCloseFlag) {
		event->ignore();
	} 
	else {
		event->accept();
	}
}

const QRect GenericViewer::getSelectionRect(void) {
	return scrollArea->getSelectionRect();
}

void GenericViewer::setSelectionTool(bool toggled) {
	scrollArea->setSelectionTool( toggled );
}

void GenericViewer::removeSelection(void) {
	scrollArea->removeSelection();
	isSelectionReady = false;
}

bool GenericViewer::hasSelection(void) {
        return isSelectionReady;
}

bool GenericViewer::needsSaving(void) {
        return NeedsSaving;
}

void GenericViewer::setNeedsSaving(bool s) {
        NeedsSaving = s;
}

const QString GenericViewer::getFileName(void) {
        return filename;
}

void GenericViewer::setFileName(const QString fn) {
        filename = fn;
}

void GenericViewer::slotCornerButtonPressed() {
	panIconWidget->setImage(image);
	float zf=scrollArea->getScaleFactor();
	float leftviewpos=(float)(scrollArea->horizontalScrollBar()->value());
	float topviewpos=(float)(scrollArea->verticalScrollBar()->value());
	float wps_w=(float)(scrollArea->maximumViewportSize().width());
	float wps_h=(float)(scrollArea->maximumViewportSize().height());
	QRect r((int)(leftviewpos/zf), (int)(topviewpos/zf), (int)(wps_w/zf), (int)(wps_h/zf));
	panIconWidget->setRegionSelection(r);
	panIconWidget->setMouseFocus();
	connect(panIconWidget, SIGNAL(signalSelectionMoved(QRect, bool)), this, SLOT(slotPanIconSelectionMoved(QRect, bool)));
	QPoint g = scrollArea->mapToGlobal(scrollArea->viewport()->pos());
	g.setX(g.x()+ scrollArea->viewport()->size().width());
	g.setY(g.y()+ scrollArea->viewport()->size().height());
	panIconWidget->popup(QPoint(g.x() - panIconWidget->width()/2, g.y() - panIconWidget->height()/2));
	panIconWidget->setCursorToLocalRegionSelectionCenter();
}

void GenericViewer::slotPanIconSelectionMoved(QRect gotopos, bool mousereleased) {
if (mousereleased) {
	scrollArea->horizontalScrollBar()->setValue((int)(gotopos.x()*scrollArea->getScaleFactor()));
	scrollArea->verticalScrollBar()->setValue((int)(gotopos.y()*scrollArea->getScaleFactor()));
		panIconWidget->close();
		slotPanIconHidden();
	}
}

void GenericViewer::slotPanIconHidden() {
	cornerButton->blockSignals(true);
	cornerButton->animateClick();
	cornerButton->blockSignals(false);
}

