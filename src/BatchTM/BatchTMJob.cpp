/**
 * This file is a part of LuminanceHDR package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2011 Davide Anastasia
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
 * Original Work
 * @author Davide Anastasia <davideanastasia@users.sourceforge.net>
 *
 */

#include "BatchTM/BatchTMJob.h"
#include "Fileformat/pfstiff.h"
#include "Exif/ExifOperations.h"
#include "Libpfs/frame.h"
#include "Filter/pfscut.h"
#include "Core/IOWorker.h"
#include "Common/LuminanceOptions.h"
#include "Core/IOWorker.h"
#include "Fileformat/pfsout16bitspixmap.h"
#include "Fileformat/pfsoutldrimage.h"
#include "TonemappingEngine/TonemapOperator.h"
#include "Filter/pfssize.h"

#include <QFileInfo>
#include <QByteArray>
#include <QDebug>
#include <QImage>
#include <QScopedPointer>

BatchTMJob::BatchTMJob(int thread_id, QString filename, const QList<TonemappingOptions*>* tm_options, QString output_folder):
        m_thread_id(thread_id),
        m_file_name(filename),
        m_tm_options(tm_options),
        m_output_folder(output_folder)
{
    m_ldr_output_format = LuminanceOptions().getBatchTmLdrFormat();
    m_ldr_output_quality = LuminanceOptions().getBatchTmDefaultOutputQuality();

    m_output_file_name_base  = m_output_folder + "/" + QFileInfo(m_file_name).completeBaseName();
}

BatchTMJob::~BatchTMJob()
{}

void BatchTMJob::run()
{
    ProgressHelper prog_helper;
    IOWorker io_worker;

    emit add_log_message(tr("[T%1] Start processing %2").arg(m_thread_id).arg(QFileInfo(m_file_name).completeBaseName()));

    // reference frame
    QScopedPointer<pfs::Frame> reference_frame( io_worker.read_hdr_frame(m_file_name) );

    if ( !reference_frame.isNull() )
    {
        // update message box
        emit add_log_message(tr("[T%1] Successfully load %2").arg(m_thread_id).arg(QFileInfo(m_file_name).completeBaseName()));

        // update progress bar!
        emit increment_progress_bar(1);

        for (int idx = 0; idx < m_tm_options->size(); ++idx)
        {
            TonemappingOptions* opts = m_tm_options->at(idx);

            opts->tonemapSelection = false; // just to be sure!
            opts->origxsize = reference_frame->getWidth();
            //opts->xsize = 400; // DEBUG
            //opts->xsize = opts->origxsize;

            QScopedPointer<pfs::Frame> temporary_frame;
            if ( reference_frame->getWidth() == opts->xsize )
                temporary_frame.reset( pfs::pfscopy(reference_frame.data()) );
            else
                temporary_frame.reset( pfs::resizeFrame(reference_frame.data(), opts->xsize) );

            QScopedPointer<TonemapOperator> tm_operator( TonemapOperator::getTonemapOperator(opts->tmoperator) );

            tm_operator->tonemapFrame(temporary_frame.data(), opts, prog_helper);

            TMOptionsOperations operations(opts);
            QString output_file_name = m_output_file_name_base+"_"+operations.getPostfix()+"."+m_ldr_output_format;

            if ( io_worker.write_ldr_frame(temporary_frame.data(), output_file_name, m_ldr_output_quality, opts) )
            {
                emit add_log_message( tr("[T%1] Successfully saved LDR file: %2").arg(m_thread_id).arg(QFileInfo(output_file_name).completeBaseName()) );
            } else {
                emit add_log_message( tr("[T%1] ERROR: Cannot save to file: %2").arg(m_thread_id).arg(QFileInfo(output_file_name).completeBaseName()) );
            }

            emit increment_progress_bar(1);
        }
    }
    else
    {
        // update message box
        //emit add_log_message(error_message);
        emit add_log_message(tr("[T%1] ERROR: Loading of %2 failed").arg(m_thread_id).arg(QFileInfo(m_file_name).completeBaseName()));

        // update progress bar!
        emit increment_progress_bar(m_tm_options->size() + 1);
    }

    emit done(m_thread_id);
}
