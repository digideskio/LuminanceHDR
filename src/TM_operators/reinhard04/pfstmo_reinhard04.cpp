/**
 * @file pfstmo_reinhard04.cpp
 * @brief Tone map XYZ channels using Reinhard04 model
 *
 * Dynamic Range Reduction Inspired by Photoreceptor Physiology.
 * E. Reinhard and K. Devlin.
 * In IEEE Transactions on Visualization and Computer Graphics, 2004.
 *
 * This file is a part of Qtpfsgui package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Grzegorz Krawczyk
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: pfstmo_reinhard04.cpp,v 1.2 2004/09/22 10:00:30 krawczyk Exp $
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include <QFile>
#include "../libpfs/pfs.h"

using namespace std;

/**
 * @brief: Tone mapping algorithm [Reinhard2004]
 *
 * @param R red channel
 * @param G green channel
 * @param B blue channel
 * @param Y luminance channel
 */
void tmo_reinhard04( pfs::Array2D* R, pfs::Array2D* G, pfs::Array2D* B, 
  pfs::Array2D* Y, float f, float w )
{
  float max_lum = (*Y)(0);
  float min_lum = (*Y)(0);
  float world_lum = 0.0;
  int im_width = Y->getCols();
  int im_height = Y->getRows();
  int im_size = im_width * im_height;

  for( int i=1 ; i<im_size ; i++ )
  {
    double lum = (*Y)(i);
    max_lum = (max_lum > lum) ? max_lum : lum;
    min_lum = (min_lum < lum) ? min_lum : lum;
    world_lum += log(2.3e-5+lum);
  }
  world_lum = exp(world_lum/im_size);

  //--- tone map image
  max_lum = log( max_lum );
  min_lum = log( min_lum );

  float k = (max_lum - log(world_lum)) / (max_lum - min_lum);
  float m = 0.3f+0.7f*pow(k,1.4f);
  f = exp(-f);

  float max_col = 0.0f;
  float min_col = 1.0f;

  int x,y;
  for( x=0 ; x<im_width ; x++ )
    for( y=0 ; y<im_height ; y++ )
    {
      float l = (*Y)(x,y);
      float col;
      if( l != 0.0f )
      {
        for( int c=0 ; c<3 ; c++ )
        {
          switch(c)
          {
          case 0: col = (*R)(x,y); break;
          case 1: col = (*G)(x,y); break;
          case 2: col = (*B)(x,y); break;
          };

          if( col!=0.0f)
          {
            float Ia = w*l + (1.0f-w)*col;
            col /= col + pow(f*Ia, m);
          }

          max_col = (col>max_col) ? col : max_col;
          min_col = (col<min_col) ? col : min_col;

          switch(c)
          {
          case 0: (*R)(x,y) = col; break;
          case 1: (*G)(x,y) = col; break;
          case 2: (*B)(x,y) = col; break;
          };
        }
      }
    }

  //--- normalize intensities
  for( x=0 ; x<im_width ; x++ )
    for( y=0 ; y<im_height ; y++ )
    {
      (*R)(x,y) = ((*R)(x,y)-min_col)/(max_col-min_col);
      (*G)(x,y) = ((*G)(x,y)-min_col)/(max_col-min_col);
      (*B)(x,y) = ((*B)(x,y)-min_col)/(max_col-min_col);
    }
}

pfs::Frame * pfstmo_reinhard04(pfs::Frame *inputpfsframe, float br, float sat ) {
	assert(inputpfsframe!=NULL);
	//--- default tone mapping parameters;
	float brightness = br;
	float saturation = sat;

	pfs::DOMIO pfsio;
	pfs::Channel *X, *Y, *Z;
	inputpfsframe->getXYZChannels(X,Y,Z);
	assert( X!=NULL && Y!=NULL && Z!=NULL );

	pfs::Frame *outframe = pfsio.createFrame( inputpfsframe->getWidth(), inputpfsframe->getHeight() );
	assert(outframe != NULL);
	pfs::Channel *Xo, *Yo, *Zo;
	outframe->createXYZChannels( Xo, Yo, Zo );
	assert( Xo!=NULL && Yo!=NULL && Zo!=NULL );

	pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, Xo, Yo, Zo ); //in:xyz ---> out:rgb (skip copyArray)
	tmo_reinhard04( Xo, Yo, Zo, Y, brightness, saturation );
	pfs::transformColorSpace( pfs::CS_RGB, Xo, Yo, Zo, pfs::CS_XYZ, Xo, Yo, Zo ); //out:rgb ---> out:xyz

	return outframe;
}